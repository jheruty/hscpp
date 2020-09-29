#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "hscpp/Platform.h"
#include "hscpp/IFileWatcher.h"
#include "hscpp/Preprocessor.h"
#include "hscpp/ICompiler.h"
#include "hscpp/ModuleManager.h"
#include "hscpp/module/AllocationResolver.h"
#include "hscpp/Feature.h"
#include "hscpp/FileParser.h"
#include "hscpp/ProtectedFunction.h"
#include "hscpp/Callbacks.h"
#include "hscpp/FeatureManager.h"
#include "hscpp/FsPathHasher.h"
#include "hscpp/Config.h"

namespace hscpp
{

    class Hotswapper
    {
    public:
        enum class UpdateResult
        {
            Idle,
            Compiling,
            StartedCompiling,
            PerformedSwap,
            FailedSwap,
        };

        Hotswapper();
        explicit Hotswapper(const Config& config);
        Hotswapper(const Config& config,
                   std::unique_ptr<IFileWatcher> pFileWatcher,
                   std::unique_ptr<ICompiler> pCompiler);

        AllocationResolver* GetAllocationResolver();

        void SetAllocator(IAllocator* pAllocator);
        void SetGlobalUserData(void* pGlobalUserData);

        void EnableFeature(Feature feature);
        void DisableFeature(Feature feature);
        bool IsFeatureEnabled(Feature feature);

        void CreateDependencyGraph();
        void TriggerManualBuild();

        UpdateResult Update();
        bool IsCompiling();

        void SetCallbacks(const Callbacks& callbacks);
        void DoProtectedCall(const std::function<void()>& cb);

        //============================================================================
        // Add & Remove Functions
        //============================================================================
        
        int AddIncludeDirectory(const fs::path& directoryPath);
        bool RemoveIncludeDirectory(int handle);
        void EnumerateIncludeDirectories(const std::function<void(int handle, const fs::path& directoryPath)>& cb);
        void ClearIncludeDirectories();

        int AddSourceDirectory(const fs::path& directoryPath);
        bool RemoveSourceDirectory(int handle);
        void EnumerateSourceDirectories(const std::function<void(int handle, const fs::path& directoryPath)>& cb);
        void ClearSourceDirectories();

        int AddForceCompiledSourceFile(const fs::path& filePath);
        bool RemoveForceCompiledSourceFile(int handle);
        void EnumerateForceCompiledSourceFiles(const std::function<void(int handle, const fs::path& directoryPath)>& cb);
        void ClearForceCompiledSourceFiles();

        int AddLibrary(const fs::path& libraryPath);
        bool RemoveLibrary(int handle);
        void EnumerateLibraries(const std::function<void(int handle, const fs::path& libraryPath)>& cb);
        void ClearLibraries();

        int AddPreprocessorDefinition(const std::string& definition);
        bool RemovePreprocessorDefinition(int handle);
        void EnumeratePreprocessorDefinitions(const std::function<void(int handle, const std::string& definition)>& cb);
        void ClearPreprocessorDefinitions();

        int AddCompileOption(const std::string& option);
        bool RemoveCompileOption(int handle);
        void EnumerateCompileOptions(const std::function<void(int handle, const std::string& option)>& cb);
        void ClearCompileOptions();

        int AddLinkOption(const std::string& option);
        bool RemoveLinkOption(int handle);
        void EnumerateLinkOptions(const std::function<void(int handle, const std::string& option)>& cb);
        void ClearLinkOptions();

        void SetHscppRequireVariable(const std::string& name, const std::string& val);

    private:
        fs::path m_HscppTempDirectoryPath;

        int m_NextIncludeDirectoryHandle = 0;
        int m_NextSourceDirectoryHandle = 0;
        int m_NextForceCompiledSourceFileHandle = 0;
        int m_NextLibraryHandle = 0;
        int m_NextPreprocessorDefinitionHandle = 0;
        int m_NextCompileOptionHandle = 0;
        int m_NextLinkOptionHandle = 0;

        fs::path m_BuildDirectoryPath;
        std::unordered_map<int, fs::path> m_IncludeDirectoryPathsByHandle;
        std::unordered_map<int, fs::path> m_SourceDirectoryPathsByHandle;
        std::unordered_map<int, fs::path> m_ForceCompiledSourceFilePathsByHandle;
        std::unordered_map<int, fs::path> m_LibraryPathsByHandle;
        std::unordered_map<int, std::string> m_PreprocessorDefinitionsByHandle;
        std::unordered_map<int, std::string> m_CompileOptionsByHandle;
        std::unordered_map<int, std::string> m_LinkOptionsByHandle;

        std::unordered_set<fs::path, FsPathHasher> m_QueuedSourceFilePaths;

        std::unordered_map<std::string, std::string> m_HscppRequireVariables;

        std::unique_ptr<IFileWatcher> m_pFileWatcher;
        std::vector<IFileWatcher::Event> m_FileEvents;

        Preprocessor m_Preprocessor;
        std::unique_ptr<ICompiler> m_pCompiler;
        ModuleManager m_ModuleManager;
        FeatureManager m_FeatureManager;

        AllocationResolver m_AllocationResolver;
        Callbacks m_Callbacks;

        Preprocessor::Input CreatePreprocessorInput(const std::vector<fs::path>& sourceFilePaths);
        Preprocessor::Output Preprocess(Preprocessor::Input& preprocessorInput);

        ICompiler::Input CreateCompilerInput(const Preprocessor::Output& preprocessorOutput);
        bool StartCompile(ICompiler::Input& compilerInput);

        bool PerformRuntimeSwap();

        bool CreateHscppTempDirectory();
        bool CreateBuildDirectory();

        void AppendDirectoryFiles(const std::unordered_map<int, fs::path>& directoryPathsByHandle,
            std::unordered_set<fs::path, FsPathHasher>& sourceFilePaths);

        template <typename T>
        int Add(const T& value, int& handle, std::unordered_map<int, T>& map);

        template <typename T>
        bool Remove(int handle, std::unordered_map<int, T>& map);

        template <typename T>
        void Enumerate(const std::function<void(int handle, const T& value)>& cb, std::unordered_map<int, T>& map);

        template <typename T>
        std::vector<T> AsVector(std::unordered_map<int, T>& map);
    };

    template <typename T>
    int Hotswapper::Add(const T& value, int& handle, std::unordered_map<int, T>& map)
    {
        int curHandle = handle++;
        map[curHandle] = value;

        return curHandle;
    }

    template <typename T>
    bool Hotswapper::Remove(int handle, std::unordered_map<int, T>& map)
    {
        auto it = map.find(handle);
        if (it == map.end())
        {
            return false;
        }

        map.erase(it);
        return true;
    }

    template <typename T>
    void Hotswapper::Enumerate(const std::function<void(int handle, const T& value)>& cb, std::unordered_map<int, T>& map)
    {
        for (const auto& handle__val : map)
        {
            cb(handle__val.first, handle__val.second);
        }
    }

    template <typename T>
    std::vector<T> Hotswapper::AsVector(std::unordered_map<int, T>& map)
    {
        std::vector<T> vec;
        for (const auto& handle__val : map)
        {
            vec.push_back(handle__val.second);
        }

        return vec;
    }

}