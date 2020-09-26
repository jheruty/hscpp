#include "hscpp/Compiler_gcc.h"
#include "hscpp/Platform.h"
#include "hscpp/Log.h"

namespace hscpp
{

    Compiler_gcc::Compiler_gcc()
        : Compiler_gcclike(Compiler_gcclike::Type::GCC)
    {}

}