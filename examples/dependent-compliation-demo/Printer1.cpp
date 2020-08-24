#include <iostream>
#include "Printer1.h"
#include "MathUtil.h"

Printer1::Printer1()
{
    std::cout << "Creating Printer1" << std::endl;
    std::cout << "MathUtil.h Add(1, 2): " << Add(1, 2) << std::endl;
    std::cout << std::endl;
}
