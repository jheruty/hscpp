#pragma once

#include "hscpp/ICompiler.h"

namespace hscpp
{

    class Compiler : public ICompiler
    {
    public:
        virtual bool StartBuild(const Input& info) override;
        virtual void Update() override;

        virtual bool IsCompiling() override;

        virtual bool HasCompiledModule() override;
        virtual fs::path PopModule() override;
    };

}