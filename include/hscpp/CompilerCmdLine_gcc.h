#pragma once

#include "hscpp/ICompilerCmdLine.h"
#include "hscpp/Config.h"

namespace hscpp
{

    class CompilerCmdLine_gcc : public ICompilerCmdLine
    {
    public:
        explicit CompilerCmdLine_gcc(const CompilerConfig& config);

        bool GenerateCommandFile(const fs::path& commandFilePath,
                                 const fs::path& moduleFilePath,
                                 const ICompiler::Input& input) override;

    private:
        CompilerConfig m_Config;
    };

}