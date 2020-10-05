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
    #include "hscpp/FileWatcher_win32.h"
    #include "hscpp/CmdShell_win32.h"
#elif defined(HSCPP_PLATFORM_APPLE)
    #include "hscpp/FileWatcher_apple.h"
    #include "hscpp/CmdShell_unix.h"
#elif defined(HSCPP_PLATFORM_UNIX)

#include "hscpp/FileWatcher_unix.h"
#include "hscpp/CmdShell_unix.h"

#endif

// Compiler code is cross-platform. For example, one may wish to run the clang compiler on Windows.
// By default hscpp will choose the compiler that was used to compile this file.
#include "hscpp/Compiler.h"
#include "hscpp/CompilerInitializeTask_msvc.h"
#include "hscpp/CompilerInitializeTask_gcc.h"
#include "hscpp/CompilerCmdLine_msvc.h"
#include "hscpp/CompilerCmdLine_gcc.h"

namespace hscpp { namespace platform
{

    //============================================================================
    // Setup
    //============================================================================
    enum class Setup
    {
        UnixPlatform_GccInitializer_GccInterface,
        Win32Platform_GccInitializer_GccInterface,
        Win32Platform_MsvcInitializer_MsvcInterface,
        Unknown,
    };

    static Setup GetCompilerSetup()
    {
#if defined(HSCPP_PLATFORM_WIN32)
    #if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
        return Setup::Win32Platform_GccInitializer_GccInterface;
    #elif defined(_MSC_VER)
        return Setup::Win32Platform_MsvcInitializer_MsvcInterface;
    #endif
#elif defined(HSCPP_PLATFORM_UNIX)
    #if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
        return Setup::UnixPlatform_GccInitializer_GccInterface;
    #endif
#endif
        return Setup::Unknown;
    }

    //============================================================================
    // FileWatcher
    //============================================================================

    std::unique_ptr<IFileWatcher> CreateFileWatcher()
    {
        return std::unique_ptr<IFileWatcher>(new FileWatcher());
    }

    //============================================================================
    // Compiler
    //============================================================================

    std::unique_ptr<ICompiler> CreateCompiler(const CompilerConfig& config /* = CompilerConfig() */)
    {
        std::unique_ptr<ICmdShellTask> pInitializeTask;
        std::unique_ptr<ICompilerCmdLine> pCompilerCmdLine;

        Setup setup = GetCompilerSetup();
        switch (setup)
        {
            case Setup::Win32Platform_MsvcInitializer_MsvcInterface:
                pInitializeTask = std::unique_ptr<ICmdShellTask>(new CompilerInitializeTask_msvc());
                pCompilerCmdLine = std::unique_ptr<ICompilerCmdLine>(new CompilerCmdLine_msvc(config));
                break;
            case Setup::Win32Platform_GccInitializer_GccInterface: // Fallthrough
            case Setup::UnixPlatform_GccInitializer_GccInterface:
                pInitializeTask = std::unique_ptr<ICmdShellTask>(new CompilerInitializeTask_gcc(config));
                pCompilerCmdLine = std::unique_ptr<ICompilerCmdLine>(new CompilerCmdLine_gcc(config));
                break;
            default:
                log::Warning() << HSCPP_LOG_PREFIX << "Could not deduce compiler, defaulting to gcc." << log::End();
                pInitializeTask = std::unique_ptr<ICmdShellTask>(new CompilerInitializeTask_gcc(config));
                pCompilerCmdLine = std::unique_ptr<ICompilerCmdLine>(new CompilerCmdLine_gcc(config));
                break;
        }

        return std::unique_ptr<ICompiler>(
                new Compiler(config, std::move(pInitializeTask), std::move(pCompilerCmdLine)));
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
            "/MP", // Build with multiple processes.
            "/EHsc", // Full support for standard C++ exception handling.
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

		// /std option is not consistent on msvc, and is only available on VS2017 and above.
		// https://docs.microsoft.com/en-us/cpp/build/reference/std-specify-language-standard-version?view=vs-2019
		if (cppStandard <= 11)
		{
#if defined(_MSC_VER)
    #if (_MSC_VER > 1900)
            options.push_back("/std:c" + std::to_string(cppStandard));
    #endif
#endif
		}
		else
		{
			options.push_back("/std:c++" + std::to_string(cppStandard));
		}

		return options;
    }

    static std::vector<std::string> GetDefaultCompileOptions_win32_gcc(int cppStandard)
    {
        return {
            "-std=c++" + std::to_string(cppStandard), // C++ standard (ex. C++17).
            "-shared", // Compile a shared library.
            "-fvisibility=hidden", // Hide code not explicitly made visible.
            "-"
#if defined(HSCPP_DEBUG)
            "-g", // Add debug info.
            "-l msvcrtd.lib", // Same as /MDd
#else
            "-l msvcrt.lib", // Same as /Md
#endif
        };
    }

    static std::vector<std::string> GetDefaultCompileOptions_unix_gcc(int cppStandard)
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
        Setup setup = GetCompilerSetup();
        switch (setup)
        {
            case Setup::Win32Platform_MsvcInitializer_MsvcInterface:
                return GetDefaultCompileOptions_msvc(cppStandard);
            case Setup::Win32Platform_GccInitializer_GccInterface:
                return GetDefaultCompileOptions_win32_gcc(cppStandard);
            case Setup::UnixPlatform_GccInitializer_GccInterface:
                return GetDefaultCompileOptions_unix_gcc(cppStandard);
            default:
                log::Warning() << HSCPP_LOG_PREFIX << "Could not deduce compiler, defaulting to gcc." << log::End();
                return GetDefaultCompileOptions_unix_gcc(cppStandard);
        }
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
#if defined(__clang__)
        return fs::path("clang++");
#elif defined(__GNUC__) || defined(__GNUG__)
        return fs::path("g++");
#elif defined(_MSC_VER)
        return fs::path("cl");
#endif
        log::Warning() << HSCPP_LOG_PREFIX
            << "Unable to deduce compiler executable. Defaulting to clang++." << log::End();
        return fs::path("clang++");
    }

    //============================================================================
    // Utilities
    //============================================================================

    void WriteDebugString(const std::wstring& str)
    {
#if defined(HSCPP_PLATFORM_WIN32)
        OutputDebugStringW(stream.str().c_str());
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
#endif

    }

#if defined(HSCPP_PLATFORM_WIN32)

    std::wstring GetErrorString(TOsError error)
    {
        if (error == ERROR_SUCCESS)
        {
            return L""; // No error.
        }

        LPWSTR buffer = nullptr;
        size_t size = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPWSTR>(&buffer),
            0,
            NULL);

        std::wstring message(buffer, size);
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

    std::wstring GetLastErrorString()
    {
        return GetErrorString(GetLastError());
    }

#elif defined(HSCPP_PLATFORM_UNIX)

    std::wstring GetErrorString(TOsError error)
    {
        std::string errorStr = strerror(errno);
        return std::wstring(errorStr.begin(), errorStr.end());
    }

    std::wstring GetLastErrorString()
    {
        return GetErrorString(errno);
    }

#endif

    std::string GetModuleExtension()
    {
#if defined(HSCPP_PLATFORM_WIN32)
        return "dll";
#elif defined(HSCPP_PLATFORM_APPLE)
        return "dynlib";
#elif defined(HSCPP_PLATFORM_UNIX)
        return "so";
#endif

        log::Warning() << HSCPP_LOG_PREFIX
            << "Unable to deduce module extension, defaulting to 'so'." << log::End();
        return "so";
    }


    void* LoadModule(const fs::path& modulePath)
    {
#if defined(HSCPP_PLATFORM_WIN32)
        return LoadLibraryW(modulePath.wstring().c_str());
#elif defined(HSCPP_PLATFORM_UNIX)
        return dlopen(modulePath.string().c_str(), RTLD_NOW);
#endif
    }

}}