#include <iostream>
#include <thread>
#include <chrono>

#include "hscpp/Hotswapper.h"
#include "hscpp/FileWatcher.h"
#include "hscpp/Constructors.h"
#include "hscpp/StringUtil.h"
#include "hscpp/SharedModuleMemory.h"

#include "Printer1.h"
#include "Printer2.h"

int main() 
{
    hscpp::Hotswapper swapper;

    swapper.AddIncludeDirectory("./include");
    swapper.AddIncludeDirectory("../../include");
    swapper.AddSourceDirectory("./src", true);

    auto p = new Printer1();
    auto p2 = new Printer2();

    std::string guid = hscpp::CreateGuid();

    //Printer p;

    while (true)
    {
        swapper.Update();
    }

    return 0;
}