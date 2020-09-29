#pragma once

#include <memory>

#include "hscpp/Filesystem.h"
#include "hscpp/ICompiler.h"

namespace hscpp
{
    struct CompilerConfig
    {
        int cppStandard = HSCPP_CXX_STANDARD;

        fs::path executable;
        fs::path startupScript;

        std::unique_ptr<ICompiler> pCustomCompiler;
    };

    struct FileWatcherConfig
    {

    };

    struct Config
    {
        CompilerConfig compiler;
    };
}