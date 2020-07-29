#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>

#include "hscpp/FileWatcher.h"
#include "hscpp/Compiler.h"
#include "hscpp/Tracker.h"
#include "hscpp/AllocationResolver.h"
#include "hscpp/ModuleManager.h"

namespace hscpp
{
    class Hotswapper
    {
    public:
        Hotswapper();

        void SetAllocator(std::unique_ptr<IAllocator> pAllocator);
        void SetGlobalUserData(void* pGlobalUserData);

        void AddIncludeDirectory(const std::filesystem::path& directory);
        void RemoveIncludeDirectory(const std::filesystem::path& directory);
        std::vector<std::filesystem::path> GetIncludeDirectories();

        void AddSourceDirectory(const std::filesystem::path& directory, bool bRecursive);
        void RemoveSourceDirectory(const std::filesystem::path& directory);
        std::vector<std::filesystem::path> GetSourceDirectories();

        void AddCompileOption(const std::string& option);
        void RemoveCompileOption(const std::string& option);
        std::vector<std::string> GetCompileOptions();

        // TODO: Add linker options, directory, and libraries.

        void Update();

        template <typename T>
        T* Allocate(uint64_t id = 0);

    private:
        std::filesystem::path m_HscppTempDirectory;

        std::filesystem::path m_BuildDirectory;
        std::vector<std::filesystem::path> m_IncludeDirectories;
        std::vector<std::filesystem::path> m_SourceDirectories;
        std::vector<std::string> m_CompileOptions;

        FileWatcher m_FileWatcher;
        std::vector<FileWatcher::Event> m_FileEvents;

        Compiler m_Compiler;
        ModuleManager m_ModuleManager;

        AllocationResolver m_AllocationResolver = AllocationResolver(&m_ModuleManager);

        std::filesystem::path GetHscppIncludePath()
        {
            // __FILE__ returns "<path>/include/hscpp/Hotswapper.h". We want "<path>/include".
            std::filesystem::path currentPath = __FILE__;
            return currentPath.parent_path().parent_path();
        }

        bool CreateHscppTempDirectory();
        bool CreateBuildDirectory();

        std::vector<std::filesystem::path> GetChangedFiles();
    };

    template <typename T>
    T* hscpp::Hotswapper::Allocate(uint64_t id /*= 0*/)
    {
        return m_AllocationResolver.Allocate<T>(id);
    }

}