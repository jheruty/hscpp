#pragma once

#include <chrono>

#include "hscpp/Filesystem.h"
#include "hscpp/ICompiler.h"
#include "hscpp/IFileWatcher.h"

namespace hscpp
{
    struct CompilerConfig
    {
        CompilerConfig();

        int cppStandard = HSCPP_CXX_STANDARD;

        fs::path executable;
        fs::path startupScript;

        std::vector<std::string> defaultCompileOptions;
        std::vector<std::string> defaultPreprocessorDefinitions;
        std::vector<fs::path> defaultIncludeDirectories;
        std::vector<fs::path> defaultForceCompiledSourceFiles;
    };

    struct FileWatcherConfig
    {
        std::chrono::milliseconds latency = std::chrono::milliseconds(100);
    };

    struct Config
    {
        CompilerConfig compiler;
        FileWatcherConfig fileWatcher;
    };
}