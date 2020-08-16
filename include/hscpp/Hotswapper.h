#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <set>
#include <unordered_map>

#include "hscpp/FileWatcher.h"
#include "hscpp/Compiler.h"
#include "hscpp/ModuleManager.h"
#include "hscpp/module/AllocationResolver.h"
#include "hscpp/Feature.h"
#include "hscpp/FileParser.h"

namespace hscpp
{
    class Hotswapper
    {
    public:
        Hotswapper(bool bUseDefaults = true);

        AllocationResolver* GetAllocationResolver();

        void SetAllocator(IAllocator* pAllocator);
        void SetGlobalUserData(void* pGlobalUserData);

        void EnableFeature(Feature feature);
        void DisableFeature(Feature feature);
        bool IsFeatureEnabled(Feature feature);

        void Update();

        //============================================================================
        // Add & Remove Functions
        //============================================================================
        
        int AddIncludeDirectory(const std::filesystem::path& directory);
        bool RemoveIncludeDirectory(int handle);
        void EnumerateIncludeDirectories(const std::function<void(int handle, const std::filesystem::path& directory)>& cb);
        void ClearIncludeDirectories();

        int AddSourceDirectory(const std::filesystem::path& directory, bool bRecursive);
        bool RemoveSourceDirectory(int handle);
        void EnumerateSourceDirectories(const std::function<void(int handle,  const std::filesystem::path& directory)>& cb);
        void ClearSourceDirectories();

        int AddLibrary(const std::filesystem::path& libraryPath);
        bool RemoveLibrary(int handle);
        void EnumerateLibraries(const std::function<void(int handle, const std::filesystem::path& libraryPath)>& cb);
        void ClearLibraries();

        int AddCompileOption(const std::string& option);
        bool RemoveCompileOption(int handle);
        void EnumerateCompileOptions(const std::function<void(int handle, const std::string& option)>& cb);
        void ClearCompileOptions();

        int AddLinkOption(const std::string& option);
        bool RemoveLinkOption(int handle);
        void EnumerateLinkOptions(const std::function<void(int handle, const std::string& option)>& cb);
        void ClearLinkOptions();

        int AddFileExtension(const std::string& extension);
        bool RemoveFileExtension(int handle);
        void EnumerateFileExtensions(const std::function<void( int handle, const std::string& option)>& cb);
        void ClearFileExtensions();

        void SetHscppRequireVariable(const std::string& name, const std::string& val);

    private:
        std::set<Feature> m_Features;
        std::filesystem::path m_HscppTempDirectory;

        int m_NextIncludeDirectoryHandle = 0;
        int m_NextSourceDirectoryHandle = 0;
        int m_NextLibraryHandle = 0;
        int m_NextCompileOptionHandle = 0;
        int m_NextLinkOptionHandle = 0;
        int m_NextFileExtensionHandle = 0;

        std::filesystem::path m_BuildDirectory;
        std::unordered_map<int, std::filesystem::path> m_IncludeDirectoriesByHandle;
        std::unordered_map<int, std::filesystem::path> m_SourceDirectoriesByHandle;
        std::unordered_map<int, std::filesystem::path> m_LibrariesByHandle;
        std::unordered_map<int, std::string> m_CompileOptionsByHandle;
        std::unordered_map<int, std::string> m_LinkOptionsByHandle;
        std::unordered_map<int, std::string> m_FileExtensionsByHandle;

        std::unordered_map<std::string, std::string> m_HscppRequireVariables;

        FileWatcher m_FileWatcher;
        std::vector<FileWatcher::Event> m_FileEvents;

        Compiler m_Compiler;
        FileParser m_FileParser;
        ModuleManager m_ModuleManager;

        AllocationResolver m_AllocationResolver;

        std::filesystem::path GetHscppIncludePath();

        bool CreateHscppTempDirectory();
        bool CreateBuildDirectory();

        std::vector<std::filesystem::path> GetChangedFiles();
        void ParseHscppRequires(Compiler::CompileInfo& compileInfo);
        void InterpolateRequireVariables(std::filesystem::path& path);

        template <typename T>
        int Add(const T& value, int& handle, std::unordered_map<int, T>& map);

        template <typename T>
        bool Remove(int handle, std::unordered_map<int, T>& map);

        template <typename T>
        void Enumerate(const std::function<void(int handle, const T& value)>& cb, std::unordered_map<int, T>& map);

        template <typename T>
        std::vector<T> AsVector(std::unordered_map<int, T>& map);
    };

    // Inline this function, so that __FILE__ is within the include directory.
    inline std::filesystem::path Hotswapper::GetHscppIncludePath()
    {
        // __FILE__ returns "<path>/include/hscpp/Hotswapper.h". We want "<path>/include".
        std::filesystem::path currentPath = __FILE__;
        return currentPath.parent_path().parent_path();
    }

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
        for (const auto& it : map)
        {
            cb(it.first, it.second);
        }
    }

    template <typename T>
    std::vector<T> Hotswapper::AsVector(std::unordered_map<int, T>& map)
    {
        std::vector<T> vec;
        for (const auto& it : map)
        {
            vec.push_back(it.second);
        }

        return vec;
    }

}