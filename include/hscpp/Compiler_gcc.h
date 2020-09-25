#pragma once

#include "hscpp/ICompiler.h"
#include "hscpp/ICmdShell.h"

namespace hscpp
{

    class Compiler_gcc : public ICompiler
    {
    public:
        Compiler_gcc();

        bool IsInitialized() override;

        bool StartBuild(const Input& info) override;
        void Update() override;

        bool IsCompiling() override;

        bool HasCompiledModule() override;
        fs::path PopModule() override;

    private:
        std::unique_ptr<ICmdShell> m_pCmdShell;
    };

}