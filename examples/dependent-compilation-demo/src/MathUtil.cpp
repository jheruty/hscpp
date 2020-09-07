#include "dependent-compilation-demo/MathUtil.h"

hscpp_module("math");

int Add(int a, int b)
{
    // Make a change here to see changes reflected in Printer1.cpp and Printer2.cpp.
    return a + b;
}
