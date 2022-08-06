#include <cstring>

#include "hscpp/Platform.h"
#include "hscpp/Log.h"

// Add includes for platform-specific OS headers.
#if defined(HSCPP_PLATFORM_WIN32)
    #include <Windows.h>
#elif defined(HSCPP_PLATFORM_UNIX)
    #include <uuid/uuid.h>
#endif

// Add includes for platform-specific hscpp classes.
#if defined(HSCPP_PLATFORM_WIN32)
    #include "hscpp/file-watcher/FileWatcher_win32.h"
    #include "hscpp/cmd-shell/CmdShell_win32.h"
#elif defined(HSCPP_PLATFORM_APPLE)
    #include "hscpp/file-watcher/FileWatcher_apple.h"
    #include "hscpp/cmd-shell/CmdShell_unix.h"
#elif defined(HSCPP_PLATFORM_UNIX)
    #include "hscpp/file-watcher/FileWatcher_unix.h"
    #include "hscpp/cmd-shell/CmdShell_unix.h"
#endif

// Compiler and GCC interface is cross-platform. MSVC interface is Win32-only.
#include "hscpp/compiler/Compiler.h"
#include "hscpp/compiler/CompilerInitializeTask_gcc.h"
#include "hscpp/compiler/CompilerCmdLine_gcc.h"

#if defined(HSCPP_PLATFORM_WIN32)
    #include "hscpp/compiler/CompilerInitializeTask_msvc.h"
    #include "hscpp/compiler/CompilerCmdLine_msvc.h"
#endif

namespace hscpp { namespace platform
{

    //============================================================================
    // FileWatcher
    //============================================================================

    std::unique_ptr<IFileWatcher> CreateFileWatcher(FileWatcherConfig* pConfig)
    {
        return std::unique_ptr<IFileWatcher>(new FileWatcher(pConfig));
    }

    //============================================================================
    // Compiler
    //============================================================================

    std::unique_ptr<ICompiler> CreateCompiler(CompilerConfig* pConfig /* = CompilerConfig() */)
    {
        std::unique_ptr<ICmdShellTask> pInitializeTask;
        std::unique_ptr<ICompilerCmdLine> pCompilerCmdLine;

#if defined(HSCPP_COMPILER_MSVC)
        pInitializeTask = std::unique_ptr<ICmdShellTask>(new CompilerInitializeTask_msvc());
        pCompilerCmdLine = std::unique_ptr<ICompilerCmdLine>(new CompilerCmdLine_msvc(pConfig));
#elif defined(HSCPP_COMPILER_CLANG_CL)
        pInitializeTask = std::unique_ptr<ICmdShellTask>(new CompilerInitializeTask_gcc(pConfig));
        pCompilerCmdLine = std::unique_ptr<ICompilerCmdLine>(new CompilerCmdLine_msvc(pConfig));
#elif defined(HSCPP_COMPILER_CLANG)
        pInitializeTask = std::unique_ptr<ICmdShellTask>(new CompilerInitializeTask_gcc(pConfig));
        pCompilerCmdLine = std::unique_ptr<ICompilerCmdLine>(new CompilerCmdLine_gcc(pConfig));
#elif defined(HSCPP_COMPILER_GCC)
        pInitializeTask = std::unique_ptr<ICmdShellTask>(new CompilerInitializeTask_gcc(pConfig));
        pCompilerCmdLine = std::unique_ptr<ICompilerCmdLine>(new CompilerCmdLine_gcc(pConfig));
#else
        log::Warning() << HSCPP_LOG_PREFIX << "Could not deduce compiler, defaulting to gcc." << log::End();
        pInitializeTask = std::unique_ptr<ICmdShellTask>(new CompilerInitializeTask_gcc(pConfig));
        pCompilerCmdLine = std::unique_ptr<ICompilerCmdLine>(new CompilerCmdLine_gcc(pConfig));
#endif

        return std::unique_ptr<ICompiler>(
                new Compiler(pConfig, std::move(pInitializeTask), std::move(pCompilerCmdLine)));
    }

    //============================================================================
    // CmdShell
    //============================================================================

    std::unique_ptr<ICmdShell> CreateCmdShell()
    {
        return std::unique_ptr<ICmdShell>(new CmdShell());
    }

    //============================================================================
    // Compile Options
    //============================================================================


    static std::vector<std::string> GetDefaultCompileOptions_msvc(int cppStandard)
    {
        std::vector<std::string> options = {
            "/nologo", // Suppress cl startup banner.
            "/Z7", // Add full debugging information.
            "/FC", // Print full filepath in diagnostic messages.
            "/EHsc", // Full support for standard C++ exception handling.
#if !defined(HSCPP_COMPILER_CLANG_CL)
            "/MP", // Build with multiple processes (not supported on clang-cl).
#endif

#if defined(HSCPP_DEBUG)
            // Debug flags.
            "/MDd", // Use multithreaded debug DLL version of run-time library.
            "/LDd", // Create debug DLL.
#else
            // Release flags.
            "/MD", // Use multithreaded release DLL version of run-time library.
            "/Zo", // Enable enhanced debugging for optimized code.
            "/LD", // Create release DLL.
#endif
        };

#if (_MSC_VER > 1900)
		// /std option is not consistent on msvc, and is only available on VS2017 and above.
		// https://docs.microsoft.com/en-us/cpp/build/reference/std-specify-language-standard-version?view=vs-2019
		if (cppStandard <= 11)
		{
            options.push_back("/std:c" + std::to_string(cppStandard));
		}
		else
		{
			options.push_back("/std:c++" + std::to_string(cppStandard));
		}
#else
		HSCPP_UNUSED_PARAM(cppStandard);
#endif

		return options;
    }

    static std::vector<std::string> GetDefaultCompileOptions_gcc(int cppStandard)
    {
        return {
            "-std=c++" + std::to_string(cppStandard), // C++ standard (ex. C++17).
            "-shared", // Compile a shared library.
            "-fPIC", // Use position-independent code.
            "-fvisibility=hidden", // Hide code not explicitly made visible.

#if defined(HSCPP_DEBUG)
            "-g", // Add debug info.
#endif
        };
    }

    std::vector<std::string> GetDefaultCompileOptions(int cppStandard /*= HSCPP_CXX_STANDARD*/)
    {

#if defined(HSCPP_COMPILER_MSVC)
        return GetDefaultCompileOptions_msvc(cppStandard);
#elif defined(HSCPP_COMPILER_CLANG_CL)
        return GetDefaultCompileOptions_msvc(cppStandard);
#elif defined(HSCPP_COMPILER_CLANG)
        return GetDefaultCompileOptions_gcc(cppStandard);
#elif defined(HSCPP_COMPILER_GCC)
        return GetDefaultCompileOptions_gcc(cppStandard);
#endif

        log::Warning() << HSCPP_LOG_PREFIX << "Could not deduce compiler, defaulting to gcc." << log::End();
        return GetDefaultCompileOptions_gcc(cppStandard);
    }

    //============================================================================
    // Preprocessor Definitions
    //============================================================================

    static std::vector<std::string> GetDefaultPreprocessorDefinitions_win32()
    {
        return {
#ifdef _DEBUG
            "_DEBUG",
#endif
#ifdef _WIN32
            "_WIN32",
#endif
        };
    }

    std::vector<std::string> GetDefaultPreprocessorDefinitions()
    {
#if defined(HSCPP_PLATFORM_WIN32)
        return GetDefaultPreprocessorDefinitions_win32();
#else
        return {};
#endif
    }

    //============================================================================
    // Compiler Executable
    //============================================================================

    fs::path GetDefaultCompilerExecutable()
    {
        return fs::path(HSCPP_COMPILER_PATH);
    }

    //============================================================================
    // Utilities
    //============================================================================

    void WriteDebugString(const std::string& str)
    {
#if defined(HSCPP_PLATFORM_WIN32)
        OutputDebugString(str.c_str());
#else
        HSCPP_UNUSED_PARAM(str);
#endif
    }

    std::string CreateGuid()
    {

#if defined(HSCPP_PLATFORM_WIN32)

        GUID guid;
    CoCreateGuid(&guid);

    char buf[64];
    snprintf(buf, sizeof(buf), "%08X-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X",
        guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2],
        guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

    return buf;

#elif defined(HSCPP_PLATFORM_UNIX)

        uuid_t uuid;
        uuid_generate_random(uuid);

        char buf[64];
        uuid_unparse(uuid, buf);

        return buf;
#else
        static_assert(false, "Unsupported platform.");
        return "";
#endif

    }

#if defined(HSCPP_PLATFORM_WIN32)

    std::string GetErrorString(TOsError error)
    {
        if (error == ERROR_SUCCESS)
        {
            return ""; // No error.
        }

        LPVOID buffer = nullptr;
        size_t size = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPTSTR>(&buffer),
            0,
            NULL);

        std::string message(static_cast<char*>(buffer), size);
        LocalFree(buffer);

        // Remove trailing '\r\n'.
        if (message.size() >= 2
            && message.at(message.size() - 2) == '\r'
            && message.at(message.size() - 1) == '\n')
        {
            message.pop_back();
            message.pop_back();
        }

        return message;
    }

    std::string GetLastErrorString()
    {
        return GetErrorString(GetLastError());
    }

#elif defined(HSCPP_PLATFORM_UNIX)

    std::string GetErrorString(TOsError error)
    {
        return strerror(error);
    }

    std::string GetLastErrorString()
    {
        return GetErrorString(errno);
    }

#endif

    std::string GetSharedLibraryExtension()
    {
#if defined(HSCPP_PLATFORM_WIN32)
        return "dll";
#elif defined(HSCPP_PLATFORM_APPLE)
        return "dynlib";
#elif defined(HSCPP_PLATFORM_UNIX)
        return "so";
#else
        static_assert(false, "Unsupported platform.");
#endif
    }


    void* LoadModule(const fs::path& modulePath)
    {
#if defined(HSCPP_PLATFORM_WIN32)
        return LoadLibraryW(modulePath.wstring().c_str());
#elif defined(HSCPP_PLATFORM_UNIX)
        return dlopen(modulePath.string().c_str(), RTLD_NOW);
#else
        static_assert(false, "Unsupported platform.");
        return nullptr;
#endif
    }

}}