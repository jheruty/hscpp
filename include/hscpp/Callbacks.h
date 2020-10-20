#pragma once

#include <functional>

#include "hscpp/compiler/ICompiler.h"

namespace hscpp
{
    struct Callbacks
    {
        std::function<void(ICompiler::Input&)> BeforeCompile;
        std::function<void()> BeforeSwap;
        std::function<void()> AfterSwap;
    };
}