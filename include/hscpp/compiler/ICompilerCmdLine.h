#pragma once

#include "hscpp/compiler/ICompiler.h"

namespace hscpp
{
    class ICompilerCmdLine
    {
    public:
        virtual ~ICompilerCmdLine() = default;

        virtual bool GenerateCommandFile(const fs::path& commandFilePath,
                                         const fs::path& moduleFilePath,
                                         const ICompiler::Input& input) = 0;
    };
}