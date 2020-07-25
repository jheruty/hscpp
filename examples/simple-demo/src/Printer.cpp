#include <iostream>

#include "hscpp/Register.h"
#include "Printer.h"

Printer::Printer()
{
    HSCPP_ON_BEFORE_SWAP([]() {
            std::cout << "About to swap!" << std::endl;
        });

    HSCPP_ON_AFTER_SWAP([]() {
            std::cout << "Swap complete!" << std::endl;
        });
}

void Printer::Update()
{
}