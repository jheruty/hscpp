#include <unordered_set>
#include <fstream>
#include <thread>
#include <chrono>
#include <assert.h>

#include "hscpp/Hotswapper.h"
#include "hscpp/Log.h"
#include "hscpp/Util.h"

namespace hscpp
{

    const static std::string HSCPP_TEMP_DIRECTORY_NAME = "HSCPP_7c9279ff-25af-488c-a634-b6aa68f47a65";

    // Users may not override these file options.
    const static std::unordered_set<std::string> FORBIDDEN_COMPILE_OPTIONS = {
        "Fe", // Name of compiled module.
        "Fo", // Name of directory to place all object files.
    };
    
    const static std::vector<std::string> DEFAULT_COMPILE_OPTIONS = {
        "/nologo", // Suppress cl startup banner.
        "/std:c++17", // Use C++17 standard.
        "/Z7", // Add full debugging information.
        "/FC", // Print full filepath in diagnostic messages.
        "/MP", // Build with multiple processes.
        "/EHsc", // Full support for standard C++ exception handling.
#ifdef _DEBUG
        // Debug flags.
        "/MDd", // Use multithreaded debug DLL version of run-time library.
        "/LDd", // Create debug DLL. 
#else
        // Release flags.
        "/MD", // Use multithreaded release DLL version of run-time library.
        "/Zo", // Enable enhanced debugging for optimized code.
        "/LD", // Create release DLL.
#endif
    };

    const static std::vector<std::string> DEFAULT_PREPROCESSOR_DEFINITIONS = {
#ifdef _DEBUG
        "_DEBUG",
#endif
    };

    Hotswapper::Hotswapper(bool bUseDefaults /* = true */)
    {
        if (bUseDefaults)
        {
            for (const auto& option : DEFAULT_COMPILE_OPTIONS)
            {
                Add(option, m_NextCompileOptionHandle, m_CompileOptionsByHandle);
            }

            for (const auto& definition : DEFAULT_PREPROCESSOR_DEFINITIONS)
            {
                Add(definition, m_NextPreprocessorDefinitionHandle, m_PreprocessorDefinitionsByHandle);
            }

            // Add hotswap-cpp include directory as a default include directory, since parts of the
            // library will need to be compiled into each new module.
            Add(GetHscppIncludePath(), m_NextIncludeDirectoryHandle, m_IncludeDirectoriesByHandle);
        }
    }

    hscpp::AllocationResolver* Hotswapper::GetAllocationResolver()
    {
        return &m_AllocationResolver;
    }

    void Hotswapper::SetAllocator(IAllocator* pAllocator)
    {
        m_ModuleManager.SetAllocator(pAllocator);
    }

    void Hotswapper::SetGlobalUserData(void* pGlobalUserData)
    {
        m_ModuleManager.SetGlobalUserData(pGlobalUserData);
    }

    void Hotswapper::EnableFeature(Feature feature)
    {
        m_Features.insert(feature);
    }

    void Hotswapper::DisableFeature(Feature feature)
    {
        auto it = m_Features.find(feature);
        if (it != m_Features.end())
        {
            m_Features.erase(it);
        }
    }

    bool Hotswapper::IsFeatureEnabled(Feature feature)
    {
        auto it = m_Features.find(feature);
        return it != m_Features.end();
    }

    void Hotswapper::CreateDependencyGraph()
    {
        Preprocessor::Input input;
        input.includeDirectories = AsVector(m_IncludeDirectoriesByHandle);
        input.sourceDirectories = AsVector(m_SourceDirectoriesByHandle);
        
        for (const auto& pair : m_SourceDirectoriesByHandle)
        {
            for (const auto& file : std::filesystem::directory_iterator(pair.second))
            {
                if (util::IsSourceFile(file))
                {
                    input.files.push_back(file);
                }
            }
        }

        for (const auto& pair : m_IncludeDirectoriesByHandle)
        {
            for (const auto& file : std::filesystem::directory_iterator(pair.second))
            {
                if (util::IsHeaderFile(file))
                {
                    input.files.push_back(file);
                }
            }
        }
        m_Preprocessor.CreateDependencyGraph(input);
    }

    void Hotswapper::TriggerManualBuild()
    {
        if (CreateBuildDirectory())
        {
            Preprocessor::Input preprocessorInput;
            preprocessorInput.bHscppMacros = IsFeatureEnabled(Feature::Preprocessor);
            preprocessorInput.bDependentCompilation = IsFeatureEnabled(Feature::DependentCompilation);

            preprocessorInput.includeDirectories = AsVector(m_IncludeDirectoriesByHandle);
            preprocessorInput.sourceDirectories = AsVector(m_SourceDirectoriesByHandle);
            preprocessorInput.libraries = AsVector(m_LibrariesByHandle);
            preprocessorInput.preprocessorDefinitions = AsVector(m_PreprocessorDefinitionsByHandle);
            preprocessorInput.hscppRequireVariables = m_HscppRequireVariables;

            if (m_Callbacks.BeforePreprocessor != nullptr)
            {
                m_Callbacks.BeforePreprocessor(preprocessorInput);
            }

            Preprocessor::Output preprocessorOutput = m_Preprocessor.Preprocess(preprocessorInput);

            if (m_Callbacks.AfterPreprocessor != nullptr)
            {
                m_Callbacks.AfterPreprocessor(preprocessorOutput);
            }

            Compiler::CompileInfo compileInfo;
            compileInfo.buildDirectory = m_BuildDirectory;
            compileInfo.files = preprocessorOutput.files;
            compileInfo.includeDirectories = preprocessorOutput.includeDirectories;
            compileInfo.libraries = preprocessorOutput.libraries;
            compileInfo.preprocessorDefinitions = preprocessorOutput.preprocessorDefinitions;
            compileInfo.compileOptions = AsVector(m_CompileOptionsByHandle);
            compileInfo.linkOptions = AsVector(m_LinkOptionsByHandle);

            if (m_Callbacks.BeforeCompile != nullptr)
            {
                m_Callbacks.BeforeCompile(compileInfo);
            }

            if (!compileInfo.files.empty())
            {
                m_Compiler.StartBuild(compileInfo);
                while (m_Compiler.IsCompiling())
                {
                    m_Compiler.Update();
                }

                if (m_Callbacks.AfterCompile != nullptr)
                {
                    m_Callbacks.AfterCompile();
                }

                if (m_Compiler.HasCompiledModule())
                {
                    PerformRuntimeSwap();
                }
            }
        }
    }

    Hotswapper::UpdateResult Hotswapper::Update()
    {
        m_Compiler.Update();
        if (m_Compiler.IsCompiling())
        {
            // Currently compiling. Let file changes queue up, to be handled after the module
            // has been swapped.
            return UpdateResult::Compiling;
        }

        if (m_Compiler.HasCompiledModule())
        {
            PerformRuntimeSwap();
            return UpdateResult::PerformedSwap;
        }

        if (!IsFeatureEnabled(Feature::ManualCompilationOnly))
        {
            m_FileWatcher.PollChanges(m_FileEvents);
        }

        if (m_FileEvents.size() > 0)
        {
            if (CreateBuildDirectory())
            {
                Preprocessor::Input preprocessorInput;
                preprocessorInput.bHscppMacros = IsFeatureEnabled(Feature::Preprocessor);
                preprocessorInput.bDependentCompilation = IsFeatureEnabled(Feature::DependentCompilation);

                preprocessorInput.files = GetChangedFiles();
                preprocessorInput.includeDirectories = AsVector(m_IncludeDirectoriesByHandle);
                preprocessorInput.sourceDirectories = AsVector(m_SourceDirectoriesByHandle);
                preprocessorInput.libraries = AsVector(m_LibrariesByHandle);
                preprocessorInput.preprocessorDefinitions = AsVector(m_PreprocessorDefinitionsByHandle);
                preprocessorInput.hscppRequireVariables = m_HscppRequireVariables;

                if (m_Callbacks.BeforePreprocessor != nullptr)
                {
                    m_Callbacks.BeforePreprocessor(preprocessorInput);
                }

                Preprocessor::Output preprocessorOutput = m_Preprocessor.Preprocess(preprocessorInput);

                if (m_Callbacks.AfterPreprocessor != nullptr)
                {
                    m_Callbacks.AfterPreprocessor(preprocessorOutput);
                }

                Compiler::CompileInfo compileInfo;
                compileInfo.buildDirectory = m_BuildDirectory;
                compileInfo.files = preprocessorOutput.files;
                compileInfo.includeDirectories = preprocessorOutput.includeDirectories;
                compileInfo.libraries = preprocessorOutput.libraries;
                compileInfo.preprocessorDefinitions = preprocessorOutput.preprocessorDefinitions;
                compileInfo.compileOptions = AsVector(m_CompileOptionsByHandle);
                compileInfo.linkOptions = AsVector(m_LinkOptionsByHandle);

                if (m_Callbacks.BeforeCompile != nullptr)
                {
                    m_Callbacks.BeforeCompile(compileInfo);
                }

                if (!compileInfo.files.empty())
                {
                    m_Compiler.StartBuild(compileInfo);

                    if (m_Callbacks.AfterCompile != nullptr)
                    {
                        m_Callbacks.AfterCompile();
                    }

                    return UpdateResult::StartedCompiling;
                }
            }
        }

        return UpdateResult::Nothing;
    }

    bool Hotswapper::IsCompiling()
    {
        return m_Compiler.IsCompiling();
    }

    void Hotswapper::SetCallbacks(const Callbacks& callbacks)
    {
        m_Callbacks = callbacks;
    }

    void Hotswapper::DoProtectedCall(const std::function<void()>& cb)
    {
#ifdef HSCPP_DISABLE
        cb();
#else
        // Attempt to call 'Protected' function. On a structured exception, rather than terminating
        // the program, let the user modify their code and attempt to fix the issue.
        ProtectedFunction::Result result = ProtectedFunction::Call(cb);
        
        // Keep recompiling user's changes until protected call succeeds.
        while (result != ProtectedFunction::Result::Success)
        {
            Log::Error() << HSCPP_LOG_PREFIX << "Failed protected call. " 
                << "Make code changes and save to reattempt." << EndLog();

            while (Update() != UpdateResult::PerformedSwap)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            result = ProtectedFunction::Call(cb);
        }
#endif
    }

    //============================================================================
    // Add & Remove Functions
    //============================================================================

    int Hotswapper::AddIncludeDirectory(const fs::path& directory)
    {
        m_FileWatcher.AddWatch(directory);
        return Add(directory, m_NextIncludeDirectoryHandle, m_IncludeDirectoriesByHandle);
    }

    bool Hotswapper::RemoveIncludeDirectory(int handle)
    {
        auto it = m_IncludeDirectoriesByHandle.find(handle);
        if (it != m_IncludeDirectoriesByHandle.end())
        {
            m_FileWatcher.RemoveWatch(it->second);
        }

        return Remove(handle, m_IncludeDirectoriesByHandle);
    }

    void Hotswapper::EnumerateIncludeDirectories(const std::function<void(int handle, const fs::path& directory)>& cb)
    {
        Enumerate(cb, m_IncludeDirectoriesByHandle);
    }

    void Hotswapper::ClearIncludeDirectories()
    {
        m_IncludeDirectoriesByHandle.clear();
    }

    int Hotswapper::AddSourceDirectory(const fs::path& directory)
    {
        m_FileWatcher.AddWatch(directory);
        return Add(directory, m_NextSourceDirectoryHandle, m_SourceDirectoriesByHandle);
    }

    bool Hotswapper::RemoveSourceDirectory(int handle)
    {
        auto it = m_SourceDirectoriesByHandle.find(handle);
        if (it != m_SourceDirectoriesByHandle.end())
        {
            m_FileWatcher.RemoveWatch(it->second);
        }

        return Remove(handle, m_SourceDirectoriesByHandle);
    }

    void Hotswapper::EnumerateSourceDirectories(const std::function<void(int handle, const fs::path& directory)>& cb)
    {
        Enumerate(cb, m_SourceDirectoriesByHandle);
    }

    void Hotswapper::ClearSourceDirectories()
    {
        m_SourceDirectoriesByHandle.clear();
    }

    int Hotswapper::AddLibrary(const fs::path& libraryPath)
    {
        return Add(libraryPath, m_NextLibraryHandle, m_LibrariesByHandle);
    }

    bool Hotswapper::RemoveLibrary(int handle)
    {
        return Remove(handle, m_LibrariesByHandle);
    }

    void Hotswapper::EnumerateLibraries(const std::function<void(int handle, const fs::path& libraryPath)>& cb)
    {
        Enumerate(cb, m_LibrariesByHandle);
    }

    void Hotswapper::ClearLibraries()
    {
        m_LibrariesByHandle.clear();
    }

    int Hotswapper::AddPreprocessorDefinition(const std::string& definition)
    {
        return Add(definition, m_NextPreprocessorDefinitionHandle, m_PreprocessorDefinitionsByHandle);
    }

    bool Hotswapper::RemovePreprocessorDefinition(int handle)
    {
        return Remove(handle, m_PreprocessorDefinitionsByHandle);
    }

    void Hotswapper::EnumeratePreprocessorDefinitions(const std::function<void(int handle, const std::string& definition)>& cb)
    {
        Enumerate(cb, m_PreprocessorDefinitionsByHandle);
    }

    void Hotswapper::ClearPreprocessorDefinitions()
    {
        m_PreprocessorDefinitionsByHandle.clear();
    }

    int Hotswapper::AddCompileOption(const std::string& option)
    {
        return Add(option, m_NextCompileOptionHandle, m_CompileOptionsByHandle);
    }

    bool Hotswapper::RemoveCompileOption(int handle)
    {
        return Remove(handle, m_CompileOptionsByHandle);
    }

    void Hotswapper::EnumerateCompileOptions(const std::function<void(int handle, const std::string& option)>& cb)
    {
        Enumerate(cb, m_CompileOptionsByHandle);
    }

    void Hotswapper::ClearCompileOptions()
    {
        m_CompileOptionsByHandle.clear();
    }

    int Hotswapper::AddLinkOption(const std::string& option)
    {
        return Add(option, m_NextLinkOptionHandle, m_LinkOptionsByHandle);
    }

    bool Hotswapper::RemoveLinkOption(int handle)
    {
        return Remove(handle, m_LinkOptionsByHandle);
    }

    void Hotswapper::EnumerateLinkOptions(const std::function<void(int handle, const std::string& option)>& cb)
    {
        Enumerate(cb, m_LinkOptionsByHandle);
    }

    void Hotswapper::ClearLinkOptions()
    {
        m_LinkOptionsByHandle.clear();
    }

    void Hotswapper::SetHscppRequireVariable(const std::string& name, const std::string& val)
    {
        m_HscppRequireVariables[name] = val;
    }

    void Hotswapper::PerformRuntimeSwap()
    {
        if (m_Callbacks.BeforeSwap != nullptr)
        {
            m_Callbacks.BeforeSwap();
        }

        m_ModuleManager.PerformRuntimeSwap(m_Compiler.PopModule());

        if (m_Callbacks.AfterSwap != nullptr)
        {
            m_Callbacks.AfterSwap();
        }
    }

    //============================================================================

    bool Hotswapper::CreateHscppTempDirectory()
    {
        std::error_code error;
        fs::path temp = fs::temp_directory_path(error);

        if (error.value() != ERROR_SUCCESS)
        {
            Log::Error() << HSCPP_LOG_PREFIX << "Failed to find temp directory path. " << ErrorLog(error) << EndLog();
            return false;
        }

        fs::path hscppTemp = temp / HSCPP_TEMP_DIRECTORY_NAME;

        fs::remove_all(hscppTemp, error);
        if (!fs::create_directory(hscppTemp, error))
        {
            Log::Error() << HSCPP_LOG_PREFIX << "Failed to create directory "
                << hscppTemp << ". " << ErrorLog(error) << EndLog();
            return false;
        }

        m_HscppTempDirectory = hscppTemp;

        return true;
    }

    bool Hotswapper::CreateBuildDirectory()
    {
        if (m_HscppTempDirectory.empty())
        {
            if (!CreateHscppTempDirectory())
            {
                return false;
            }
        }

        std::string guid = util::CreateGuid();
        m_BuildDirectory = m_HscppTempDirectory / guid;

        std::error_code error;
        if (!fs::create_directory(m_BuildDirectory, error))
        {
            Log::Error() << HSCPP_LOG_PREFIX << "Failed to create directory "
                << m_BuildDirectory << ErrorLog(error) << EndLog();
            return false;
        }

        return true;
    }

    std::vector<fs::path> Hotswapper::GetChangedFiles()
    {
        // When Visual Studio saves, it can create several events for a single file, so use a
        // set to remove these duplicates.
        std::unordered_set<std::wstring> files;
        for (const auto& event : m_FileEvents)
        {
            if (event.type == FileWatcher::EventType::Removed)
            {
                // Removed files don't matter, we simply will not attempt to compile them.
                continue;
            }

            std::error_code error;
            fs::path canonicalPath = fs::canonical(event.filepath, error);

            if (error.value() == ERROR_FILE_NOT_FOUND)
            {
                // While saving a file, temporary copies may be created and then removed. Skip them.
                continue;
            }
            else if (error.value() != ERROR_SUCCESS)
            {
                Log::Error() << HSCPP_LOG_PREFIX << "Failed to get canonical path of "
                    << event.filepath << ". " << ErrorLog(error) << EndLog();
                continue;
            }

            if (!util::IsHeaderFile(canonicalPath) && !util::IsSourceFile(canonicalPath))
            {
                Log::Info() << HSCPP_LOG_PREFIX << "File " << canonicalPath
                    << " will be skipped; its extension is not being watched." << EndLog();
                continue;
            }

            switch (event.type)
            {
            case FileWatcher::EventType::Added:
            case FileWatcher::EventType::Modified:
                files.insert(canonicalPath.wstring());
                break;
            default:
                assert(false);
                break;
            }
        }

        return std::vector<fs::path>(files.begin(), files.end());
    }

}
