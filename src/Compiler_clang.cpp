#include "hscpp/Compiler_clang.h"
#include "hscpp/Platform.h"
#include "hscpp/Log.h"

namespace hscpp
{

    Compiler_clang::Compiler_clang()
        : Compiler_gcclike(Compiler_gcclike::Type::Clang)
    {}

}