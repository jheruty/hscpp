#pragma once

#include "hscpp/compiler/ICompilerCmdLine.h"
#include "hscpp/Config.h"

namespace hscpp
{

    class CompilerCmdLine_gcc : public ICompilerCmdLine
    {
    public:
        explicit CompilerCmdLine_gcc(CompilerConfig* pConfig);

        bool GenerateCommandFile(const fs::path& commandFilePath,
                                 const fs::path& moduleFilePath,
                                 const ICompiler::Input& input) override;

    private:
        CompilerConfig* m_pConfig = nullptr;
    };

}