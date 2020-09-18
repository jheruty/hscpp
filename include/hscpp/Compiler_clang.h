#pragma once

#include "hscpp/ICompiler.h"
#include "hscpp/ICmdShell.h"

namespace hscpp
{

    class Compiler_clang : public ICompiler
    {
    public:
        Compiler_clang();

        bool StartBuild(const Input& info) override;
        void Update() override;

        bool IsCompiling() override;

        bool HasCompiledModule() override;
        fs::path PopModule() override;

    private:
        std::unique_ptr<ICmdShell> m_pCmdShell;
    };

}