#pragma once

#include <memory>

#include "hscpp/IFileWatcher.h"
#include "hscpp/ICompiler.h"
#include "hscpp/ICmdShell.h"
#include "hscpp/Filesystem.h"

#ifdef HSCPP_PLATFORM_WIN32

#include <Windows.h>

#elif HSCPP_PLATFORM_UNIX

#include <errno.h>

#endif

namespace hscpp
{

#ifdef HSCPP_PLATFORM_WIN32

typedef unsigned long TOsError;

#define HSCPP_ERROR_SUCCESS ERROR_SUCCESS
#define HSCPP_ERROR_FILE_NOT_FOUND ERROR_FILE_NOT_FOUND

#elif HSCPP_PLATFORM_UNIX

typedef int TOsError;

// TODO
#define HSCPP_ERROR_SUCCESS 0
#define HSCPP_ERROR_FILE_NOT_FOUND ENOENT

#else
#endif

    namespace platform
    {
        std::unique_ptr<IFileWatcher> CreateFileWatcher();
        std::unique_ptr<ICompiler> CreateCompiler();
        std::unique_ptr<ICmdShell> CreateCmdShell();
    }

}