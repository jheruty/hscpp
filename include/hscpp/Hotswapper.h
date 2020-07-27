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

        void AddCompileOption(const Compiler::CompileOption& option);
        void RemoveCompileOption(const Compiler::CompileOption& option);
        std::vector<Compiler::CompileOption> GetCompileOptions();

        void Update();
    private:
        std::filesystem::path m_HscppTempDirectory;

        std::filesystem::path m_BuildDirectory;
        std::vector<std::filesystem::path> m_IncludeDirectories;
        std::vector<std::filesystem::path> m_SourceDirectories;
        std::vector<Compiler::CompileOption> m_CompileOptions;

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