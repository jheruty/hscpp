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

    Hotswapper::Hotswapper()
        : Hotswapper(std::unique_ptr<Config>(new Config()), nullptr, nullptr)
    {}

    Hotswapper::Hotswapper(std::unique_ptr<Config> pConfig)
        : Hotswapper(std::move(pConfig), nullptr, nullptr)
    {}

    Hotswapper::Hotswapper(std::unique_ptr<Config> pConfig,
                           std::unique_ptr<IFileWatcher> pFileWatcher,
                           std::unique_ptr<ICompiler> pCompiler)
       : m_pConfig(std::move(pConfig))
    {
        if (pFileWatcher != nullptr)
        {
            m_pFileWatcher = std::move(pFileWatcher);
        }
        else
        {
            m_pFileWatcher = platform::CreateFileWatcher(&m_pConfig->fileWatcher);
        }

        if (pCompiler != nullptr)
        {
            m_pCompiler = std::move(pCompiler);
        }
        else
        {
            m_pCompiler = platform::CreateCompiler(&m_pConfig->compiler);
        }

        if (!(m_pConfig->flags & Config::Flag::NoDefaultCompileOptions))
        {
            for (const auto &option : platform::GetDefaultCompileOptions())
            {
                Add(option, m_NextCompileOptionHandle, m_CompileOptionsByHandle);
            }
        }

        if (!(m_pConfig->flags & Config::Flag::NoDefaultPreprocessorDefinitions))
        {
            for (const auto &definition : platform::GetDefaultPreprocessorDefinitions())
            {
                Add(definition, m_NextPreprocessorDefinitionHandle, m_PreprocessorDefinitionsByHandle);
            }
        }

        if (!(m_pConfig->flags & Config::Flag::NoDefaultIncludeDirectories))
        {
            // Add hotswap-cpp include directory as a default include directory, since parts of the
            // library will need to be compiled into each new module.
            Add(util::GetHscppIncludePath(), m_NextIncludeDirectoryHandle, m_IncludeDirectoryPathsByHandle);
        }

        if (!(m_pConfig->flags & Config::Flag::NoDefaultForceCompiledSourceFiles))
        {
            fs::path moduleFilePath = util::GetHscppSourcePath() / "module" / "Module.cpp";
            Add(moduleFilePath, m_NextForceCompiledSourceFileHandle, m_ForceCompiledSourceFilePathsByHandle);
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

    void Hotswapper::TriggerManualBuild()
    {
        if (CreateBuildDirectory())
        {
            ICompiler::Input compilerInput = CreateCompilerInput({});

            if (StartCompile(compilerInput))
            {
                while (m_pCompiler->IsCompiling())
                {
                    m_pCompiler->Update();
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }

                if (m_pCompiler->HasCompiledModule())
                {
                    PerformRuntimeSwap();
                }
            }
        }
    }

    Hotswapper::UpdateResult Hotswapper::Update()
    {
        if (m_bDependencyGraphNeedsRefresh && IsFeatureEnabled(Feature::DependentCompilation))
        {
            RefreshDependencyGraph();
        }

        m_pCompiler->Update();
        if (m_pCompiler->IsCompiling())
        {
            // Currently compiling. Let file changes queue up, to be handled after the module
            // has been swapped.
            return UpdateResult::Compiling;
        }

        if (m_pCompiler->HasCompiledModule())
        {
            if (PerformRuntimeSwap())
            {
                return UpdateResult::PerformedSwap;
            }
            else
            {
                return UpdateResult::FailedSwap;
            }
        }

        if (!IsFeatureEnabled(Feature::ManualCompilationOnly))
        {
            m_pFileWatcher->PollChanges(m_FileEvents);
        }

        if (!m_FileEvents.empty())
        {
            if (CreateBuildDirectory())
            {
                std::vector<fs::path> canonicalModifiedFiles;
                std::vector<fs::path> canonicalRemovedFiles;
                util::SortFileEvents(m_FileEvents, canonicalModifiedFiles, canonicalRemovedFiles);

                RemoveFromDependencyGraph(canonicalRemovedFiles);
                if (!canonicalModifiedFiles.empty())
                {
                    ICompiler::Input compilerInput = CreateCompilerInput(canonicalModifiedFiles);
                    if (StartCompile(compilerInput))
                    {
                        return UpdateResult::StartedCompiling;
                    }
                }
            }
        }

        return UpdateResult::Idle;
    }

    bool Hotswapper::IsCompiling()
    {
        return m_pCompiler->IsCompiling();
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
        m_bDependencyGraphNeedsRefresh = true;

        m_pFileWatcher->AddWatch(directoryPath);
        return Add(directoryPath, m_NextIncludeDirectoryHandle, m_IncludeDirectoryPathsByHandle);
    }

    bool Hotswapper::RemoveIncludeDirectory(int handle)
    {
        auto it = m_IncludeDirectoryPathsByHandle.find(handle);
        if (it != m_IncludeDirectoryPathsByHandle.end())
        {
            m_pFileWatcher->RemoveWatch(it->second);
        }

        bool bRemoved = Remove(handle, m_IncludeDirectoryPathsByHandle);
        if (bRemoved)
        {
            m_bDependencyGraphNeedsRefresh = true;
        }

        return bRemoved;
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
        m_bDependencyGraphNeedsRefresh = true;

        m_pFileWatcher->AddWatch(directoryPath);
        return Add(directoryPath, m_NextSourceDirectoryHandle, m_SourceDirectoryPathsByHandle);
    }

    bool Hotswapper::RemoveSourceDirectory(int handle)
    {
        auto it = m_SourceDirectoryPathsByHandle.find(handle);
        if (it != m_SourceDirectoryPathsByHandle.end())
        {
            m_pFileWatcher->RemoveWatch(it->second);
        }

        bool bRemoved = Remove(handle, m_SourceDirectoryPathsByHandle);
        if (bRemoved)
        {
            m_bDependencyGraphNeedsRefresh = true;
        }

        return bRemoved;
    }

    void Hotswapper::EnumerateSourceDirectories(const std::function<void(int handle, const fs::path& directoryPath)>& cb)
    {
        Enumerate(cb, m_SourceDirectoryPathsByHandle);
    }

    void Hotswapper::ClearSourceDirectories()
    {
        m_SourceDirectoryPathsByHandle.clear();
    }

    int Hotswapper::AddForceCompiledSourceFile(const fs::path& filePath)
    {
        return Add(filePath, m_NextForceCompiledSourceFileHandle, m_ForceCompiledSourceFilePathsByHandle);
    }

    bool Hotswapper::RemoveForceCompiledSourceFile(int handle)
    {
        return Remove(handle, m_ForceCompiledSourceFilePathsByHandle);
    }

    void Hotswapper::EnumerateForceCompiledSourceFiles(
            const std::function<void(int handle, const fs::path& directoryPath)>& cb)
    {
        Enumerate(cb, m_ForceCompiledSourceFilePathsByHandle);
    }

    void Hotswapper::ClearForceCompiledSourceFiles()
    {
        m_ForceCompiledSourceFilePathsByHandle.clear();
    }

    int Hotswapper::AddLibraryDirectory(const fs::path& directoryPath)
    {
         return Add(directoryPath, m_NextLibraryDirectoryHandle, m_LibraryDirectoryPathsByHandle);
    }

    bool Hotswapper::RemoveLibraryDirectory(int handle)
    {
        return Remove(handle, m_LibraryDirectoryPathsByHandle);
    }

    void Hotswapper::EnumerateLibraryDirectories(const std::function<void(int, const fs::path&)>& cb)
    {
        Enumerate(cb, m_LibraryDirectoryPathsByHandle);
    }

    void Hotswapper::ClearLibraryDirectories()
    {
        m_LibraryDirectoryPathsByHandle.clear();
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

    void Hotswapper::SetVar(const std::string& name, const std::string& val)
    {
        m_VarManager.SetVar(name, val);
    }

    bool Hotswapper::RemoveVar(const std::string& name)
    {
        return m_VarManager.RemoveVar(name);
    }

    //============================================================================

    bool Hotswapper::StartCompile(ICompiler::Input& compilerInput)
    {
        if (m_Callbacks.BeforeCompile != nullptr)
        {
            m_Callbacks.BeforeCompile(compilerInput);
        }

        if (!compilerInput.sourceFilePaths.empty())
        {
            if (m_pCompiler->StartBuild(compilerInput))
            {
                return true;
            }
        }

        return false;
    }

    ICompiler::Input Hotswapper::CreateCompilerInput(const std::vector<fs::path>& sourceFilePaths)
    {
        ICompiler::Input input;
        input.buildDirectoryPath = m_BuildDirectoryPath;
        input.sourceFilePaths = sourceFilePaths;
        input.includeDirectoryPaths = AsVector(m_IncludeDirectoryPathsByHandle);
        input.libraryDirectoryPaths = AsVector(m_LibraryDirectoryPathsByHandle);
        input.libraryPaths = AsVector(m_LibraryPathsByHandle);
        input.preprocessorDefinitions = AsVector(m_PreprocessorDefinitionsByHandle);
        input.compileOptions = AsVector(m_CompileOptionsByHandle);
        input.linkOptions = AsVector(m_LinkOptionsByHandle);

        for (const auto& handle__filePath : m_ForceCompiledSourceFilePathsByHandle)
        {
            input.sourceFilePaths.push_back(handle__filePath.second);
        }

        Deduplicate(input);
        Preprocess(input);
        Deduplicate(input);

        return input;
    }

    void Hotswapper::Preprocess(ICompiler::Input& input)
    {
        std::vector<fs::path> additionalSourceFilePaths;
        std::vector<fs::path> additionalIncludeDirectoryPaths;
        std::vector<fs::path> additionalLibraryDirectoryPaths;
        std::vector<fs::path> additionalLibraryPaths;
        std::vector<std::string> additionalPreprocessorDefinitions;

        if (IsFeatureEnabled(Feature::Preprocessor))
        {
            std::vector<FileParser::ParseInfo> parseInfos = m_FileParser.Parse(input.sourceFilePaths);
            for (const auto& parseInfo : parseInfos)
            {
                for (const auto& require : parseInfo.requires)
                {
                    for (const auto& path : require.paths)
                    {
                        // If path is relative, it should be relative to the path of the parse source file.
                        fs::path fullPath = path;
                        if (path.is_relative())
                        {
                            fullPath = parseInfo.filePath.parent_path() / path;
                        }

                        // Interpolate vars (ex. ${VARIABLE_NAME}).
                        fullPath = fs::u8path(m_VarManager.Interpolate(fullPath.u8string()));

                        std::error_code error;
                        fs::path canonicalPath = fs::canonical(fullPath, error);

                        if (error.value() == HSCPP_ERROR_SUCCESS)
                        {
                            switch (require.type)
                            {
                                case FileParser::Require::Type::Source:
                                    additionalSourceFilePaths.push_back(canonicalPath);
                                    break;
                                case FileParser::Require::Type::Include:
                                    additionalIncludeDirectoryPaths.push_back(canonicalPath);
                                    break;
                                case FileParser::Require::Type::LibraryDirectory:
                                    additionalLibraryDirectoryPaths.push_back(canonicalPath);
                                    break;
                                case FileParser::Require::Type::Library:
                                    additionalLibraryPaths.push_back(canonicalPath);
                                    break;
                                default:
                                    assert(false);
                                    break;
                            }
                        }
                        else
                        {
                            log::Error() << HSCPP_LOG_PREFIX << "Failed to get canonical path of "
                                         << fullPath << ". " << log::OsError(error) << log::End();
                        }
                    }
                }

                for (const auto& preprocessorDefinition : parseInfo.preprocessorDefinitions)
                {
                    additionalPreprocessorDefinitions.push_back(preprocessorDefinition);
                }
            }

            if (IsFeatureEnabled(Feature::DependentCompilation))
            {
                for (const auto& sourceFilePath : input.sourceFilePaths)
                {
                    std::vector<fs::path> dependentFilePaths = m_DependencyGraph.ResolveGraph(sourceFilePath);
                    additionalSourceFilePaths.insert(additionalSourceFilePaths.end(),
                            dependentFilePaths.begin(), dependentFilePaths.end());
                }
            }
        }

        input.sourceFilePaths.insert(input.sourceFilePaths.end(),
                additionalSourceFilePaths.begin(), additionalSourceFilePaths.end());
        input.includeDirectoryPaths.insert(input.includeDirectoryPaths.end(),
                additionalIncludeDirectoryPaths.begin(), additionalIncludeDirectoryPaths.end());
        input.libraryDirectoryPaths.insert(input.libraryDirectoryPaths.end(),
                additionalLibraryDirectoryPaths.begin(), additionalLibraryDirectoryPaths.end());
        input.libraryPaths.insert(input.libraryPaths.end(),
                additionalLibraryPaths.begin(), additionalLibraryPaths.end());
        input.preprocessorDefinitions.insert(input.preprocessorDefinitions.end(),
                additionalPreprocessorDefinitions.begin(), additionalPreprocessorDefinitions.end());
    }

    void Hotswapper::Deduplicate(ICompiler::Input& input)
    {
        util::Deduplicate<fs::path, FsPathHasher>(input.sourceFilePaths);
        util::Deduplicate<fs::path, FsPathHasher>(input.includeDirectoryPaths);
        util::Deduplicate<fs::path, FsPathHasher>(input.libraryPaths);
        util::Deduplicate(input.preprocessorDefinitions);
        util::Deduplicate(input.compileOptions);
        util::Deduplicate(input.linkOptions);
    }

    bool Hotswapper::PerformRuntimeSwap()
    {
        if (m_Callbacks.BeforeSwap != nullptr)
        {
            m_Callbacks.BeforeSwap();
        }

        bool bResult = m_ModuleManager.PerformRuntimeSwap(m_pCompiler->PopModule());

        if (m_Callbacks.AfterSwap != nullptr)
        {
            m_Callbacks.AfterSwap();
        }

        return bResult;
    }

    bool Hotswapper::CreateHscppTempDirectory()
    {
        std::error_code error;
        fs::path temp = fs::temp_directory_path(error);

        if (error.value() != HSCPP_ERROR_SUCCESS)
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

        std::string guid = platform::CreateGuid();
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

    void Hotswapper::RemoveFromDependencyGraph(const std::vector<fs::path>& removedFilePaths)
    {
        if (IsFeatureEnabled(Feature::DependentCompilation))
        {
            for (const auto& removedFilePath : removedFilePaths)
            {
                m_DependencyGraph.RemoveFile(removedFilePath);
            }
        }
    }

    void Hotswapper::RefreshDependencyGraph()
    {
        if (IsFeatureEnabled(Feature::DependentCompilation))
        {
            // Header and source directories may overlap, so collect unique files using a set.
            std::unordered_set<fs::path, FsPathHasher> uniqueSourceFilePaths;

            AppendDirectoryFiles(m_SourceDirectoryPathsByHandle, uniqueSourceFilePaths);
            AppendDirectoryFiles(m_IncludeDirectoryPathsByHandle, uniqueSourceFilePaths);

            m_DependencyGraph.Clear();

            for (const auto& sourceFilePath : uniqueSourceFilePaths)
            {
                FileParser::ParseInfo parseInfo = m_FileParser.Parse(sourceFilePath);
                std::error_code error;
                fs::path canonicalFilePath = fs::canonical(parseInfo.filePath, error);

                if (error.value() != HSCPP_ERROR_SUCCESS)
                {
                    log::Error() << HSCPP_LOG_PREFIX << "Failed to get canonical path of "
                                 << canonicalFilePath << ". " << log::OsError(error) << log::End();
                    return;
                }

                m_DependencyGraph.SetLinkedModules(canonicalFilePath, parseInfo.modules);

                std::vector<fs::path> canonicalIncludePaths;
                for (const auto& includePath : parseInfo.includePaths)
                {
                    for (const auto& includeDirectoryPath : AsVector(m_IncludeDirectoryPathsByHandle))
                    {
                        // For example, the includePath may be "MathUtil.h", but we want to find the full
                        // for creating the dependency graph. Iterate through our include directories to
                        // find the folder that contains a matching include.
                        fs::path fullIncludePath = includeDirectoryPath / includePath;
                        if (fs::exists(fullIncludePath))
                        {
                            fullIncludePath = fs::canonical(fullIncludePath, error);
                            if (error.value() == HSCPP_ERROR_SUCCESS)
                            {
                                canonicalIncludePaths.push_back(fullIncludePath);
                            }
                        }
                    }
                }

                m_DependencyGraph.SetFileDependencies(canonicalFilePath, canonicalIncludePaths);
            }
        }

        m_bDependencyGraphNeedsRefresh = false;
    }

    void Hotswapper::AppendDirectoryFiles(const std::map<int, fs::path>& directoryPathsByHandle,
        std::unordered_set<fs::path, FsPathHasher>& sourceFilePaths)
    {
        for (const auto& handle__directoryPath : directoryPathsByHandle)
        {
            std::error_code error;
            auto directoryIterator = fs::directory_iterator(handle__directoryPath.second, error);

            if (error.value() != HSCPP_ERROR_SUCCESS)
            {
                log::Error() << HSCPP_LOG_PREFIX << "Unable to iterate directory "
                    << handle__directoryPath.second << log::End(".");
                return;
            }

            for (const auto& filePath : fs::directory_iterator(handle__directoryPath.second))
            {
                if (util::IsSourceFile(filePath) || util::IsHeaderFile(filePath))
                {
                    fs::path canonicalFilePath = fs::canonical(filePath, error);
                    if (error.value() == HSCPP_ERROR_SUCCESS)
                    {
                        sourceFilePaths.insert(filePath);
                    }
                }
            }
        }
    }

}