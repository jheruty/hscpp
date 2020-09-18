#include "hscpp/Platform.h"
#include "hscpp/Log.h"

#if defined(HSCPP_PLATFORM_WIN32)

#include "hscpp/FileWatcher_win32.h"
#include "hscpp/CmdShell_win32.h"

// FileWatcher and CmdShell have platform-specific code.
#elif defined(HSCPP_PLATFORM_APPLE)

#include "hscpp/FileWatcher_apple.h"
#include "hscpp/CmdShell_unix.h"

#elif defined(HSCPP_PLATFORM_UNIX)

#include "hscpp/FileWatcher_unix.h"
#include "hscpp/CmdShell_unix.h"

#endif

// Compiler code is cross-platform. For example, one may wish to run the clang compiler on Windows.
// By default hscpp will choose the compiler that was used to compile this file.
#include "hscpp/Compiler_msvc.h"
#include "hscpp/Compiler_clang.h"
#include "hscpp/Compiler_gcc.h"

namespace hscpp
{

    std::unique_ptr<IFileWatcher> platform::CreateFileWatcher()
    {
        return std::unique_ptr<IFileWatcher>(new FileWatcher());
    }

    std::unique_ptr<ICompiler> platform::CreateCompiler()
    {
#if defined(__clang__)
        // Use clang.
        return std::unique_ptr<ICompiler>(new Compiler_clang());
#elif defined(__GNUC__) || defined(__GNUG__)
        // Use GCC.
        return std::unique_ptr<ICompiler>(new Compiler_gcc());
#elif defined(_MSC_VER)
        // Use MSVC.
        return std::unique_ptr<ICompiler>(new Compiler_msvc());
#endif
        // Unknown compiler, default to clang.
        log::Warning() << "Unknown compiler, defaulting to clang." << log::End();
        return std::unique_ptr<ICompiler>(new Compiler_clang());
    }

    std::unique_ptr<ICmdShell> platform::CreateCmdShell()
    {
        return std::unique_ptr<ICmdShell>(new CmdShell());
    }

}