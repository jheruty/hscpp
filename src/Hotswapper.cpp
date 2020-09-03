#include <unordered_set>
#include <fstream>
#include <thread>
#include <chrono>
#include <assert.h>
#include <functional>

#include "hscpp/Hotswapper.h"
#include "hscpp/Log.h"
#include "hscpp/Util.h"

namespace hscpp
{

    const static std::string HSCPP_TEMP_DIRECTORY_NAME = "HSCPP_7c9279ff-25af-488c-a634-b6aa68f47a65";

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
#ifdef _WIN32
        "_WIN32",
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
            Add(GetHscppIncludePath(), m_NextIncludeDirectoryHandle, m_IncludeDirectoryPathsByHandle);
        }

        m_Preprocessor.SetFeatureManager(&m_FeatureManager);
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
        m_FeatureManager.EnableFeature(feature);
    }

    void Hotswapper::DisableFeature(Feature feature)
    {
        m_FeatureManager.DisableFeature(feature);
    }

    bool Hotswapper::IsFeatureEnabled(Feature feature)
    {
        return m_FeatureManager.IsFeatureEnabled(feature);
    }

    void Hotswapper::CreateDependencyGraph()
    {
        // Header and source directories may overlap, so collect unique files using a set.
        std::unordered_set<fs::path, FsPathHasher> uniqueSourceFilePaths;
        
        AppendDirectoryFiles(m_SourceDirectoryPathsByHandle, uniqueSourceFilePaths);
        AppendDirectoryFiles(m_IncludeDirectoryPathsByHandle, uniqueSourceFilePaths);

        Preprocessor::Input preprocessorInput = CreatePreprocessorInput(
            std::vector(uniqueSourceFilePaths.begin(), uniqueSourceFilePaths.end()));
        m_Preprocessor.CreateDependencyGraph(preprocessorInput);
    }

    void Hotswapper::TriggerManualBuild()
    {
        if (CreateBuildDirectory())
        {
            Preprocessor::Input preprocessorInput = CreatePreprocessorInput({});
            Preprocessor::Output preprocessorOutput = Preprocess(preprocessorInput);

            Compiler::Input compilerInput = CreateCompilerInput(preprocessorOutput);
            
            if (StartCompile(compilerInput))
            {
                while (m_Compiler.IsCompiling())
                {
                    m_Compiler.Update();
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
            if (PerformRuntimeSwap())
            {
                // Successfully performed runtime swap. No files remain in queue.
                m_QueuedSourceFilePaths.clear();
                return UpdateResult::PerformedSwap;
            }
            else
            {
                return UpdateResult::FailedSwap;
            }
        }

        if (!IsFeatureEnabled(Feature::ManualCompilationOnly))
        {
            m_FileWatcher.PollChanges(m_FileEvents);
        }

        if (m_FileEvents.size() > 0)
        {
            HandleRemovedFiles();

            if (CreateBuildDirectory())
            {
                std::vector<fs::path> changedFiles = GetChangedFiles();

                Preprocessor::Input preprocessorInput = CreatePreprocessorInput(changedFiles);
                Preprocessor::Output preprocessorOutput = Preprocess(preprocessorInput);

                Compiler::Input compilerInput = CreateCompilerInput(preprocessorOutput);
                if (StartCompile(compilerInput))
                {
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
            log::Error() << HSCPP_LOG_PREFIX << "Failed protected call. "
                << "Make code changes and save to reattempt." << log::End();

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

    int Hotswapper::AddIncludeDirectory(const fs::path& directoryPath)
    {
        m_FileWatcher.AddWatch(directoryPath);
        return Add(directoryPath, m_NextIncludeDirectoryHandle, m_IncludeDirectoryPathsByHandle);
    }

    bool Hotswapper::RemoveIncludeDirectory(int handle)
    {
        auto it = m_IncludeDirectoryPathsByHandle.find(handle);
        if (it != m_IncludeDirectoryPathsByHandle.end())
        {
            m_FileWatcher.RemoveWatch(it->second);
        }

        return Remove(handle, m_IncludeDirectoryPathsByHandle);
    }

    void Hotswapper::EnumerateIncludeDirectories(const std::function<void(int handle, const fs::path& directoryPath)>& cb)
    {
        Enumerate(cb, m_IncludeDirectoryPathsByHandle);
    }

    void Hotswapper::ClearIncludeDirectories()
    {
        m_IncludeDirectoryPathsByHandle.clear();
    }

    int Hotswapper::AddSourceDirectory(const fs::path& directoryPath)
    {
        m_FileWatcher.AddWatch(directoryPath);
        return Add(directoryPath, m_NextSourceDirectoryHandle, m_SourceDirectoryPathsByHandle);
    }

    bool Hotswapper::RemoveSourceDirectory(int handle)
    {
        auto it = m_SourceDirectoryPathsByHandle.find(handle);
        if (it != m_SourceDirectoryPathsByHandle.end())
        {
            m_FileWatcher.RemoveWatch(it->second);
        }

        return Remove(handle, m_SourceDirectoryPathsByHandle);
    }

    void Hotswapper::EnumerateSourceDirectories(const std::function<void(int handle, const fs::path& directoryPath)>& cb)
    {
        Enumerate(cb, m_SourceDirectoryPathsByHandle);
    }

    void Hotswapper::ClearSourceDirectories()
    {
        m_SourceDirectoryPathsByHandle.clear();
    }

    int Hotswapper::AddLibrary(const fs::path& libraryPath)
    {
        return Add(libraryPath, m_NextLibraryHandle, m_LibraryPathsByHandle);
    }

    bool Hotswapper::RemoveLibrary(int handle)
    {
        return Remove(handle, m_LibraryPathsByHandle);
    }

    void Hotswapper::EnumerateLibraries(const std::function<void(int handle, const fs::path& libraryPath)>& cb)
    {
        Enumerate(cb, m_LibraryPathsByHandle);
    }

    void Hotswapper::ClearLibraries()
    {
        m_LibraryPathsByHandle.clear();
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

    Preprocessor::Input Hotswapper::CreatePreprocessorInput(const std::vector<fs::path>& sourceFilePaths)
    {
        Preprocessor::Input preprocessorInput;
        preprocessorInput.sourceFilePaths = sourceFilePaths;
        preprocessorInput.includeDirectoryPaths = AsVector(m_IncludeDirectoryPathsByHandle);
        preprocessorInput.sourceDirectoryPaths = AsVector(m_SourceDirectoryPathsByHandle);
        preprocessorInput.libraryPaths = AsVector(m_LibraryPathsByHandle);
        preprocessorInput.preprocessorDefinitions = AsVector(m_PreprocessorDefinitionsByHandle);
        preprocessorInput.hscppRequireVariables = m_HscppRequireVariables;

        return preprocessorInput;
    }

    Preprocessor::Output Hotswapper::Preprocess(Preprocessor::Input& preprocessorInput)
    {
        if (m_Callbacks.BeforePreprocessor != nullptr)
        {
            m_Callbacks.BeforePreprocessor(preprocessorInput);
        }

        Preprocessor::Output preprocessorOutput = m_Preprocessor.Preprocess(preprocessorInput);

        if (m_Callbacks.AfterPreprocessor != nullptr)
        {
            m_Callbacks.AfterPreprocessor(preprocessorOutput);
        }

        return preprocessorOutput;
    }

    Compiler::Input Hotswapper::CreateCompilerInput(const Preprocessor::Output& preprocessorOutput)
    {
        Compiler::Input compilerInput;
        compilerInput.buildDirectoryPath = m_BuildDirectoryPath;
        compilerInput.sourceFilePaths = preprocessorOutput.sourceFilePaths;
        compilerInput.includeDirectoryPaths = preprocessorOutput.includeDirectoryPaths;
        compilerInput.libraryPaths = preprocessorOutput.libraryPaths;
        compilerInput.preprocessorDefinitions = preprocessorOutput.preprocessorDefinitions;
        compilerInput.compileOptions = AsVector(m_CompileOptionsByHandle);
        compilerInput.linkOptions = AsVector(m_LinkOptionsByHandle);

        return compilerInput;
    }

    bool Hotswapper::StartCompile(Compiler::Input& compilerInput)
    {
        if (m_Callbacks.BeforeCompile != nullptr)
        {
            m_Callbacks.BeforeCompile(compilerInput);
        }

        if (!compilerInput.sourceFilePaths.empty())
        {
            if (m_Compiler.StartBuild(compilerInput))
            {
                return true;
            }
        }

        return false;
    }

    bool Hotswapper::PerformRuntimeSwap()
    {
        if (m_Callbacks.BeforeSwap != nullptr)
        {
            m_Callbacks.BeforeSwap();
        }

        bool bResult = m_ModuleManager.PerformRuntimeSwap(m_Compiler.PopModule());

        if (m_Callbacks.AfterSwap != nullptr)
        {
            m_Callbacks.AfterSwap();
        }

        return bResult;
    }

    //============================================================================

    bool Hotswapper::CreateHscppTempDirectory()
    {
        std::error_code error;
        fs::path temp = fs::temp_directory_path(error);

        if (error.value() != ERROR_SUCCESS)
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to find temp directory path. "
                << log::OsError(error) << log::End();
            return false;
        }

        fs::path hscppTemp = temp / HSCPP_TEMP_DIRECTORY_NAME;

        fs::remove_all(hscppTemp, error);
        if (!fs::create_directory(hscppTemp, error))
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to create directory "
                << hscppTemp << ". " << log::OsError(error) << log::End();
            return false;
        }

        m_HscppTempDirectoryPath = hscppTemp;

        return true;
    }

    bool Hotswapper::CreateBuildDirectory()
    {
        if (m_HscppTempDirectoryPath.empty())
        {
            if (!CreateHscppTempDirectory())
            {
                return false;
            }
        }

        std::string guid = util::CreateGuid();
        m_BuildDirectoryPath = m_HscppTempDirectoryPath / guid;

        std::error_code error;
        if (!fs::create_directory(m_BuildDirectoryPath, error))
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to create directory "
                << m_BuildDirectoryPath << ". " << log::OsError(error) << log::End();
            return false;
        }

        return true;
    }

    void Hotswapper::HandleRemovedFiles()
    {
        // Check if any files were removed.
        auto removeEventIt = std::find_if(m_FileEvents.begin(), m_FileEvents.end(),
            [](const FileWatcher::Event& event) {
            return event.type == FileWatcher::EventType::Removed;
        });

        if (removeEventIt != m_FileEvents.end())
        {
            if (util::IsSourceFile(removeEventIt->filePath) || util::IsHeaderFile(removeEventIt->filePath))
            {
                // At least one file was removed. Prune files that no longer exist from the tree.
                // This requires an exhaustive search, since we cannot get the canonical path to
                // a deleted file.
                m_Preprocessor.PruneDeletedFilesFromDependencyGraph();
            }
        }
    }

    std::vector<fs::path> Hotswapper::GetChangedFiles()
    {
        // When Visual Studio saves, it can create several events for a single file, so use a
        // set to remove these duplicates. Start with the queued source file paths, from previous
        // failed compilations.
        std::unordered_set<fs::path, FsPathHasher> uniqueFilePaths = m_QueuedSourceFilePaths;
        for (const auto& event : m_FileEvents)
        {
            if (event.type == FileWatcher::EventType::Removed)
            {
                // Removed files don't matter, we simply will not attempt to compile them.
                continue;
            }

            std::error_code error;
            if (!fs::is_regular_file(event.filePath, error))
            {
                // For example, a directory. Skip silently.
                continue;
            }

            fs::path canonicalFilePath = fs::canonical(event.filePath, error);

            if (error.value() == ERROR_FILE_NOT_FOUND)
            {
                // While saving a file, temporary copies may be created and then removed. Skip them.
                continue;
            }
            else if (error.value() != ERROR_SUCCESS)
            {
                log::Error() << HSCPP_LOG_PREFIX << "Failed to get canonical path of "
                    << event.filePath << ". " << log::OsError(error) << log::End();
                continue;
            }

            if (!util::IsHeaderFile(canonicalFilePath) && !util::IsSourceFile(canonicalFilePath))
            {
                log::Info() << HSCPP_LOG_PREFIX << "File " << canonicalFilePath
                    << " will be skipped; its extension is not being watched." << log::End();
                continue;
            }

            switch (event.type)
            {
            case FileWatcher::EventType::Added:
            case FileWatcher::EventType::Modified:
                uniqueFilePaths.insert(canonicalFilePath);
                break;
            default:
                assert(false);
                break;
            }
        }

        // Add files to queue, in case compilation fails.
        // TODO: Reenable queue once bugs are worked out.
        //m_QueuedSourceFilePaths.insert(uniqueFilePaths.begin(), uniqueFilePaths.end());

        return std::vector<fs::path>(uniqueFilePaths.begin(), uniqueFilePaths.end());
    }

    void Hotswapper::AppendDirectoryFiles(const std::unordered_map<int, fs::path>& directoryPathsByHandle,
        std::unordered_set<fs::path, FsPathHasher>& sourceFilePaths)
    {
        for (const auto& [handle, directoryPath] : directoryPathsByHandle)
        {
            for (const auto& filePath : std::filesystem::directory_iterator(directoryPath))
            {
                if (util::IsSourceFile(filePath) || util::IsHeaderFile(filePath))
                {
                    std::error_code error;
                    fs::path canonicalFilePath = fs::canonical(filePath, error);

                    if (error.value() == ERROR_SUCCESS)
                    {
                        sourceFilePaths.insert(filePath);
                    }
                }
            }
        }
    }

}
