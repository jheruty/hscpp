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
        FileWatcher m_FileWatcher;
        std::vector<FileWatcher::Event> m_FileEvents;

        Compiler m_Compiler;

        std::vector<std::filesystem::path> m_IncludeDirectories;
        std::unordered_map<std::string, std::vector<ITracker*>> m_TrackersByKey;

        std::vector<std::filesystem::path> GetChangedFiles();
    };
}