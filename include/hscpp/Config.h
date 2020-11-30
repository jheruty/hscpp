#pragma once

#include <chrono>
#include <memory>

#include "hscpp/Filesystem.h"
#include "hscpp/compiler/ICompiler.h"
#include "hscpp/file-watcher/IFileWatcher.h"

namespace hscpp
{
    struct CompilerConfig
    {
        CompilerConfig();

        int cppStandard = HSCPP_CXX_STANDARD;
        std::chrono::milliseconds initializeTimeout = std::chrono::milliseconds(10000);

        fs::path executable;
    };

    struct FileWatcherConfig
    {
        std::chrono::milliseconds latency = std::chrono::milliseconds(100);
    };

    struct Config
    {
        enum class Flag : uint64_t
        {
            None = 0,
            NoDefaultCompileOptions = (1 << 0),
            NoDefaultPreprocessorDefinitions = (1 << 1),
            NoDefaultIncludeDirectories = (1 << 2),
            NoDefaultForceCompiledSourceFiles = (1 << 3),
        };

        CompilerConfig compiler;
        FileWatcherConfig fileWatcher;

        Flag flags = Flag::None;
    };

    Config::Flag operator|(Config::Flag lhs, Config::Flag rhs);
    Config::Flag operator|=(Config::Flag& lhs, Config::Flag rhs);
    bool operator&(Config::Flag lhs, Config::Flag rhs);
}