#pragma once

#include "hscpp/Compiler_gcclike.h"

namespace hscpp
{

    class Compiler_gcc : public Compiler_gcclike
    {
    public:
        Compiler_gcc();
    };

}