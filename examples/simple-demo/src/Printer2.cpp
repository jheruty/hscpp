#include <iostream>

#include "hscpp/Tracker.h"
#include "Printer2.h"
#include "Printer1.h"

Printer2::Printer2()
{
    HSCPP_ON_BEFORE_SWAP([]() {
        std::cout << "About to swap Printer2!" << std::endl;
        });

    HSCPP_ON_AFTER_SWAP([]() {
        std::cout << "Swap Printer2 complete!" << std::endl;
        });

    std::cout << "Here I a!";
    std::cout << "I works!";
}

void Printer2::Update()
{

}