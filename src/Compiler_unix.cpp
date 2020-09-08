#include "hscpp/Compiler_unix.h"

namespace hscpp
{

    bool Compiler::StartBuild(const Input& info)
    {
        return false;
    }

    void Compiler::Update()
    {

    }

    bool Compiler::IsCompiling()
    {
        return false;
    }

    bool Compiler::HasCompiledModule()
    {
        return false;
    }

    fs::path Compiler::PopModule()
    {
        return fs::path();
    }


}