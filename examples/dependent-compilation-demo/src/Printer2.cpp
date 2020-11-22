#include <iostream>

#include "dependent-compilation-demo/Printer2.h"
#include "dependent-compilation-demo/MathUtil.h" // Go to this file to see how to use hscpp_module
#include "dependent-compilation-demo/StringUtil.h" // This also uses hscpp_module

Printer2::Printer2()
{
    // Saving this file will recompile both the "math" and "string" modules.
    std::cout << "Creating Printer2" << std::endl;
    std::cout << "MathUtil.h Add(1, 2): " << Add(1, 2) << std::endl;
    std::cout << "MathUtil.h Subtract(1, 2): " << Subtract(1, 2) << std::endl;
    std::cout << "StringUtil.h Concat(\"a\", \"b\"): " << Concat("a", "b") << std::endl;
    std::cout << std::endl;
}
