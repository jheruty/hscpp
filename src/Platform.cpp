#include "hscpp/Platform.h"

#ifdef HSCPP_PLATFORM_WIN32

#include "hscpp/FileWatcher_win32.h"
#include "hscpp/Compiler_win32.h"
#include "hscpp/CmdShell_win32.h"

#elif HSCPP_PLATFORM_APPLE

#include "hscpp/FileWatcher_apple.h"
#include "hscpp/Compiler_unix.h"
#include "hscpp/CmdShell_unix.h"

#elif HSCPP_PLATFORM_UNIX

#include "hscpp/FileWatcher_unix.h"
#include "hscpp/Compiler_unix.h"
#include "hscpp/CmdShell_unix.h"

#endif

namespace hscpp
{

    std::unique_ptr<IFileWatcher> platform::CreateFileWatcher()
    {
        return std::make_unique<FileWatcher>();
    }

    std::unique_ptr<ICompiler> platform::CreateCompiler()
    {
        return std::make_unique<Compiler>();
    }

    std::unique_ptr<ICmdShell> platform::CreateCmdShell()
    {
        return std::make_unique<CmdShell>();
    }

}