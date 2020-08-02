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

        void SetAllocator(IAllocator* pAllocator);
        void SetGlobalUserData(void* pGlobalUserData);

        void AddIncludeDirectory(const std::filesystem::path& directory);
        void AddSourceDirectory(const std::filesystem::path& directory, bool bRecursive);

        void AddCompileOption(const std::string& option);
        void SetCompileOptions(const std::vector<std::string>& options);
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

        std::filesystem::path GetHscppIncludePath();

        bool CreateHscppTempDirectory();
        bool CreateBuildDirectory();

        std::vector<std::filesystem::path> GetChangedFiles();
    };

    // Inline this function, so that __FILE__ is within the include directory.
    inline std::filesystem::path Hotswapper::GetHscppIncludePath()
    {
        // __FILE__ returns "<path>/include/hscpp/Hotswapper.h". We want "<path>/include".
        std::filesystem::path currentPath = __FILE__;
        return currentPath.parent_path().parent_path();
    }

    template <typename T>
    T* hscpp::Hotswapper::Allocate(uint64_t id /*= 0*/)
    {
        return m_AllocationResolver.Allocate<T>(id);
    }

}