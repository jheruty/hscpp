#pragma once

#include "hscpp/Compiler_gcclike.h"

namespace hscpp
{

    class Compiler_clang : public Compiler_gcclike
    {
    public:
        Compiler_clang();
    };

}