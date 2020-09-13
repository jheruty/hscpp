#pragma once

#include "hscpp/ICompiler.h"

namespace hscpp
{

    class Compiler : public ICompiler
    {
    public:
        bool StartBuild(const Input& info) override;
        void Update() override;

        bool IsCompiling() override;

        bool HasCompiledModule() override;
        fs::path PopModule() override;
    };

}