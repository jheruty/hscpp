#pragma once

#include "hscpp/ICompilerCmdLine.h"

namespace hscpp
{

    class CompilerCmdLine_msvc : public ICompilerCmdLine
    {
    public:
        bool GenerateCommandFile(const fs::path& commandFilePath,
                                 const fs::path& moduleFilePath,
                                 const ICompiler::Input& input) override;
    };

}