#pragma once

#include <memory>
#include <functional>

#include "hscpp/IFileWatcher.h"
#include "hscpp/ICompiler.h"
#include "hscpp/ICmdShell.h"
#include "hscpp/Filesystem.h"
#include "hscpp/Config.h"

#if defined(HSCPP_PLATFORM_WIN32)

#include <Windows.h>

#elif defined(HSCPP_PLATFORM_UNIX)

#include <errno.h>
#include <dlfcn.h>

#endif

namespace hscpp
{

#if defined(HSCPP_PLATFORM_WIN32)

typedef unsigned long TOsError;

#define HSCPP_ERROR_SUCCESS ERROR_SUCCESS
#define HSCPP_ERROR_FILE_NOT_FOUND ERROR_FILE_NOT_FOUND

#elif defined(HSCPP_PLATFORM_UNIX)

typedef int TOsError;

#define HSCPP_ERROR_SUCCESS 0
#define HSCPP_ERROR_FILE_NOT_FOUND ENOENT

#endif

    namespace platform
    {
        std::unique_ptr<IFileWatcher> CreateFileWatcher();
        std::unique_ptr<ICompiler> CreateCompiler(const CompilerConfig& config = CompilerConfig());
        std::unique_ptr<ICmdShell> CreateCmdShell();

        std::vector<std::string> GetDefaultCompileOptions(int cppStandard = HSCPP_CXX_STANDARD);
        std::vector<std::string> GetDefaultPreprocessorDefinitions();

        fs::path GetDefaultCompilerExecutable();

        void* LoadModule(const fs::path& modulePath);

        template <typename TSignature>
        std::function<TSignature> GetModuleFunction(void* pModule, const std::string& name)
        {
#if defined(HSCPP_PLATFORM_WIN32)
            return std::function<TSignature>(
                    reinterpret_cast<TSignature*>(
                            GetProcAddress(static_cast<HMODULE>(pModule), name.c_str())));
#else
            return std::function<TSignature>(
                    reinterpret_cast<TSignature*>(
                            dlsym(pModule, name.c_str())));
#endif
        }
    }

#define HSCPP_UNUSED_PARAM(param) (void)param

}