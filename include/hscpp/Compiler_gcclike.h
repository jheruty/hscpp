#pragma once

#include <memory>
#include <string>
#include <vector>

#include "hscpp/Platform.h"
#include "hscpp/ICompiler.h"
#include "hscpp/ICmdShell.h"

namespace hscpp
{

    // Compilers may have GCC-like interfaces (ex. clang). To avoid creating a separate compiler
    // for each, GCC-like compilers can inherit from this class.
    class Compiler_gcclike : public ICompiler
    {
    public:
        explicit Compiler_gcclike(const std::string& executable);

        bool IsInitialized() override;

        bool StartBuild(const Input& input) override;
        void Update() override;

        bool IsCompiling() override;

        bool HasCompiledModule() override;
        fs::path PopModule() override;

    private:
        enum class CompilerTask
        {
            GetVersion,
            Build,
        };

        bool m_bInitialized = false;

        std::unique_ptr<ICmdShell> m_pCmdShell;

        size_t m_iCompileOutput = 0;
        fs::path m_CompilingModulePath;
        fs::path m_CompiledModulePath;

        std::string m_Executable;

        bool CreateCommandFile(const Input& input);

        bool HandleTaskComplete(CompilerTask task);
        bool HandleGetVersionTaskComplete(const std::vector<std::string>& output);
        bool HandleBuildTaskComplete();
    };

}