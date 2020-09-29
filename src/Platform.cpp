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
#include "hscpp/Compiler_gcclike.h"
#include "hscpp/Compiler.h"
#include "hscpp/CompilerInitializeTask_msvc.h"
#include "hscpp/CompilerInitializeTask_gcc.h"
#include "hscpp/CompilerCmdLine_msvc.h"
#include "hscpp/CompilerCmdLine_gcc.h"

namespace hscpp { namespace platform
{

    std::unique_ptr<IFileWatcher> CreateFileWatcher()
    {
        return std::unique_ptr<IFileWatcher>(new FileWatcher());
    }

    std::unique_ptr<ICompiler> CreateCompiler()
    {
#if defined(__clang__)
        // Use clang.
        return std::unique_ptr<ICompiler>(new Compiler_gcclike("clang++"));
#elif defined(__GNUC__) || defined(__GNUG__)
        // Use GCC.
        return std::unique_ptr<ICompiler>(new Compiler_gcclike("g++"));
#elif defined(_MSC_VER)
        // Use MSVC.
        auto pInitializeTask = std::unique_ptr<ICmdShellTask>(new CompilerInitializeTask_msvc());
        auto pCompilerCmdLine = std::unique_ptr<ICompilerCmdLine>(new CompilerCmdLine_msvc());
        return std::unique_ptr<ICompiler>(
                new Compiler("cl", "", std::move(pInitializeTask), std::move(pCompilerCmdLine)));
#else
        // Unknown compiler, default to clang.
        log::Warning() << HSCPP_LOG_PREFIX << "Unknown compiler, defaulting to clang." << log::End();
        return std::unique_ptr<ICompiler>(new Compiler_gcclike("clang++"));
#endif
    }

    std::unique_ptr<ICompiler> CreateCompiler(const std::string& executable)
    {
        if (executable.empty())
        {
            return CreateCompiler();
        }

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
        // Use gcc-like compiler.
        return std::unique_ptr<ICompiler>(new Compiler_gcclike(executable));
#elif defined(_MSC_VER)
        // Use MSVC. The executable will be discovered dynamically in the Compiler.
        auto pInitializeTask = std::unique_ptr<ICmdShellTask>(new CompilerInitializeTask_msvc());
        auto pCompilerCmdLine = std::unique_ptr<ICompilerCmdLine>(new CompilerCmdLine_msvc());
        return std::unique_ptr<ICompiler>(new Compiler(executable, "", std::move(pInitializeTask), std::move(pCompilerCmdLine)));
#endif

        // Unknown compiler, use default...
        return CreateCompiler();
    }


        std::unique_ptr<ICmdShell> CreateCmdShell()
    {
        return std::unique_ptr<ICmdShell>(new CmdShell());
    }

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

    static std::vector<std::string> GetDefaultCompileOptions_gcclike(int cppStandard)
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
#if defined(__clang__)
        // Using clang.
        return GetDefaultCompileOptions_gcclike(cppStandard);
#elif defined(__GNUC__) || defined(__GNUG__)
        // Using GCC.
        return GetDefaultCompileOptions_gcclike(cppStandard);
#elif defined(_MSC_VER)
        // Using MSVC.
        return GetDefaultCompileOptions_msvc(cppStandard);
#endif
    }

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

    void* LoadModule(const fs::path& modulePath)
    {
#if defined(HSCPP_PLATFORM_WIN32)
        return LoadLibraryW(modulePath.wstring().c_str());
#elif defined(HSCPP_PLATFORM_UNIX)
        return dlopen(modulePath.string().c_str(), RTLD_NOW);
#endif
    }

}}