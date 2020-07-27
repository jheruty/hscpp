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
        void AddSourceDirectory(const std::filesystem::path& directory, bool bRecursive);

        void Update();
    private:
        std::filesystem::path m_HscppTempDirectory;

        FileWatcher m_FileWatcher;
        std::vector<FileWatcher::Event> m_FileEvents;

        Compiler m_Compiler;
        Compiler::CompileInfo m_CompileInfo;

        std::unordered_map<std::string, std::vector<ITracker*>> m_TrackersByKey;

        bool CreateHscppTempDirectory();
        bool CreateBuildDirectory(std::filesystem::path& buildDirectory);

        std::vector<std::filesystem::path> GetChangedFiles();

        bool PerformRuntimeSwap(const std::filesystem::path& moduleFilepath);
    };
}