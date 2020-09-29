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
#include "hscpp/Compiler.h"
#include "hscpp/CompilerInitializeTask_msvc.h"
#include "hscpp/CompilerInitializeTask_gcc.h"
#include "hscpp/CompilerCmdLine_msvc.h"
#include "hscpp/CompilerCmdLine_gcc.h"

namespace hscpp { namespace platform
{

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

#if defined(HSCPP_PLATFORM_WIN32)

#if defined(__clang__)
        // Using clang on Windows, use clang-cl.
        pInitializeTask = std::unique_ptr<ICmdShellTask>(new CompilerInitializeTask_gcc(config));
        pCompilerCmdLine = std::unique_ptr<ICompilerCmdLine>(new CompilerCmdLine_msvc(config));
#elif defined(_MSC_VER)
        // Using msvc on Windows.
        pInitializeTask = std::unique_ptr<ICmdShellTask>(new CompilerInitializeTask_msvc());
        pCompilerCmdLine = std::unique_ptr<ICompilerCmdLine>(new CompilerCmdLine_msvc(config));
#endif

#elif defined(HSCPP_PLATFORM_UNIX)

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
        // Using clang or g++ on UNIX.
        pInitializeTask = std::unique_ptr<ICmdShellTask>(new CompilerInitializeTask_gcc(config));
        pCompilerCmdLine = std::unique_ptr<ICompilerCmdLine>(new CompilerCmdLine_gcc(config));
#endif

#endif

        if (pInitializeTask == nullptr || pCompilerCmdLine == nullptr)
        {
            log::Error() << HSCPP_LOG_PREFIX << "Could not deduce compiler, defaulting to gcc." << log::End();
            pInitializeTask = std::unique_ptr<ICmdShellTask>(new CompilerInitializeTask_gcc(config));
            pCompilerCmdLine = std::unique_ptr<ICompilerCmdLine>(new CompilerCmdLine_gcc(config));
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
        return {
            "/nologo", // Suppress cl startup banner.
            "/std:c++" + std::to_string(cppStandard), // C++ standard (ex. C++17).
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
#if defined(HSCPP_PLATFORM_WIN32)

#if defined(__clang__) || defined(_MSC_VER)
        // On Windows, clang also uses msvc options with clang-cl.
        return GetDefaultCompileOptions_msvc(cppStandard);
#endif

#elif defined(HSCPP_PLATFORM_UNIX)

        #if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
        // Using clang or g++ on UNIX.
        return GetDefaultCompileOptions_gcc(cppStandard);
#endif

#endif

        log::Error() << HSCPP_LOG_PREFIX << "Failed to deduce compile options. Defaulting to GCC." << log::End();
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
#if defined(HSCPP_PLATFORM_WIN32)

#if defined(__clang__)
        // Using clang on Windows, use clang-cl.
        return fs::path("clang-cl");
#elif defined(_MSC_VER)
        // Using msvc on Windows.
        return fs::path("cl");
#endif

#elif defined(HSCPP_PLATFORM_UNIX)

#if defined(__clang__)
        // Using clang on UNIX.
        return fs::path("clang++");
#elif defined(__GNUC__) || defined(__GNUG__)
        // Using g++ on UNIX.
        return fs::path("g++");
#endif

#endif
        log::Error() << HSCPP_LOG_PREFIX
            << "Unable to deduce compiler executable. Defaulting to clang++." << log::End();

        return fs::path("clang++");
    }

    //============================================================================
    // Load Module
    //============================================================================

    void* LoadModule(const fs::path& modulePath)
    {
#if defined(HSCPP_PLATFORM_WIN32)
        return LoadLibraryW(modulePath.wstring().c_str());
#elif defined(HSCPP_PLATFORM_UNIX)
        return dlopen(modulePath.string().c_str(), RTLD_NOW);
#endif
    }

}}