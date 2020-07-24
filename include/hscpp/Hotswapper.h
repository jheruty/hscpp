#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "hscpp/FileWatcher.h"
#include "hscpp/Compiler.h"

namespace hscpp
{
    class Hotswapper
    {
    public:
        void AddIncludeDirectory(const std::filesystem::path& directory);
        void AddSourceDirectory(const std::filesystem::path& directory, bool bRecursive);

        void Update();
    private:
        FileWatcher m_FileWatcher;
        std::vector<FileWatcher::Event> m_FileEvents;

        Compiler m_Compiler;

        std::vector<std::filesystem::path> m_IncludeDirectories;

        std::vector<std::filesystem::path> GetChangedFiles();
    };
}