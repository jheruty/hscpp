#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>

#include "hscpp/FileWatcher.h"
#include "hscpp/Compiler.h"
#include "hscpp/Tracker.h"

namespace hscpp
{
    class Hotswapper
    {
    public:
        Hotswapper();

        void AddIncludeDirectory(const std::filesystem::path& directory);
        void RemoveIncludeDirectory(const std::filesystem::path& directory);
        std::vector<std::filesystem::path> GetIncludeDirectories();

        void AddSourceDirectory(const std::filesystem::path& directory, bool bRecursive);
        void RemoveSourceDirectory(const std::filesystem::path& directory);
        std::vector<std::filesystem::path> GetSourceDirectories();

        void AddCompileOption(const std::string& option);
        void RemoveCompileOption(const std::string& option);
        std::vector<std::string> GetCompileOptions();

        void Update();
    private:
        std::filesystem::path m_HscppTempDirectory;

        std::filesystem::path m_BuildDirectory;
        std::vector<std::filesystem::path> m_IncludeDirectories;
        std::vector<std::filesystem::path> m_SourceDirectories;
        std::vector<std::string> m_CompileOptions;

        FileWatcher m_FileWatcher;
        std::vector<FileWatcher::Event> m_FileEvents;

        Compiler m_Compiler;

        std::unordered_map<std::string, std::vector<ITracker*>> m_TrackersByKey;

        bool CreateHscppTempDirectory();
        bool CreateBuildDirectory();

        std::vector<std::filesystem::path> GetChangedFiles();

        bool PerformRuntimeSwap(const std::filesystem::path& moduleFilepath);
    };
}