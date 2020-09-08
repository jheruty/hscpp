#pragma once

#include <functional>

#include "hscpp/ICompiler.h"
#include "hscpp/Preprocessor.h"

namespace hscpp
{
    struct Callbacks
    {
        std::function<void()> BeforeSwap;
        std::function<void()> AfterSwap;

        std::function<void(Preprocessor::Input&)> BeforePreprocessor;
        std::function<void(Preprocessor::Output&)> AfterPreprocessor;

        std::function<void(ICompiler::Input&)> BeforeCompile;
    };
}