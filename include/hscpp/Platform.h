#pragma once

#include <memory>

#include "hscpp/IFileWatcher.h"
#include "hscpp/ICompiler.h"
#include "hscpp/ICmdShell.h"
#include "hscpp/Filesystem.h"

namespace hscpp
{

#ifdef HSCPP_PLATFORM_WIN32
    typedef unsigned long TOsError;
#elif HSCPP_PLATFORM_UNIX
    typedef int TOsError;
#else
#endif

    namespace platform
    {
        std::unique_ptr<IFileWatcher> CreateFileWatcher();
        std::unique_ptr<ICompiler> CreateCompiler();
        std::unique_ptr<ICmdShell> CreateCmdShell();
    }

}