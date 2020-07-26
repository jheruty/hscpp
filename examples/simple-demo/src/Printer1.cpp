#include <iostream>

#include "hscpp/Register.h"
#include "Printer1.h"

Printer1::Printer1()
{
    HSCPP_ON_BEFORE_SWAP([]() {
        std::cout << "About to swap Printer1!" << std::endl;
        });

    HSCPP_ON_AFTER_SWAP([]() {
        std::cout << "Swap Printer1 complete!" << std::endl;
        });
}

void Printer1::Update()
{
}