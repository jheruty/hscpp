#include <iostream>
#include "dependent-compilation-demo/Printer1.h"
#include "dependent-compilation-demo/MathUtil.h" // Go to this file, to see how to use hscpp_module

Printer1::Printer1()
{
    std::cout << "Creating Printer1" << std::endl;
    std::cout << "MathUtil.h Add(1, 2): " << Add(1, 2) << std::endl;
    std::cout << std::endl;
}
