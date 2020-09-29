#include "hscpp/CompilerCmdLine_gcc.h"

namespace hscpp
{

    bool CompilerCmdLine_gcc::GenerateCommandFile(const fs::path &commandFilePath,
                                                  const fs::path& moduleFilePath,
                                                  const ICompiler::Input &input)
    {
        return false;
    }

}