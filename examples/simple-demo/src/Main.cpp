#include <iostream>
#include <thread>
#include <chrono>

#include "hscpp/Hotswapper.h"
#include "hscpp/FileWatcher.h"
#include "hscpp/Constructors.h"
#include "Printer.h"
#include "hscpp/StringUtil.h"

int main() 
{
    hscpp::Hotswapper swapper;

    swapper.AddIncludeDirectory("./include");
    swapper.AddIncludeDirectory("../../include");
    swapper.AddSourceDirectory("./src", true);

    Printer* pPrinter = static_cast<Printer*>(hscpp::Constructors::Create("Printer"));

    std::string guid = hscpp::CreateGuid();

    Printer p;

    while (true)
    {
        swapper.Update();
    }

    return 0;
}