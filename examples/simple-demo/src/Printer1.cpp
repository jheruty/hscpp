#include <iostream>

#include "hscpp/Tracker.h"
#include "Printer1.h"

Printer1::Printer1()
{
    //HSCPP_ON_BEFORE_SWAP([]() {
    //    std::cout << "About to swap Printer1!" << std::endl;
    //    });

    //HSCPP_ON_AFTER_SWAP([]() {
    //    std::cout << "Swap Printer1 complete!" << std::endl;
    //    });

    std::cout << "CONSTRUCTING";
}

Printer1::~Printer1()
{
    std::cout << "DESTRUCTING";
}

void Printer1::Update()
{
    std::cout << "It works!";
}