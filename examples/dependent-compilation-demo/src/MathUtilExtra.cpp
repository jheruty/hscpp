#include "hscpp/module/PreprocessorMacros.h"
#include "dependent-compilation-demo/MathUtil.h"

hscpp_module("math");

int Subtract(int a, int b)
{
    return a - b;
}
