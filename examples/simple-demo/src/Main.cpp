#include <iostream>
#include <thread>
#include <chrono>

#include "hscpp/Hotswapper.h"
#include "hscpp/FileWatcher.h"
#include "hscpp/Constructors.h"
#include "hscpp/Swappable.h"
#include "Printer.h"
#include "hscpp/StringUtil.h"

int main() 
{
    hscpp::Hotswapper swapper;

    swapper.AddIncludeDirectory("./include");
    swapper.AddIncludeDirectory("../../include");
    swapper.AddSourceDirectory("./src", true);

    hscpp::ISwappable* pSwappable = hscpp::Constructors::Create(hscpp::Swappable<Printer>::s_Key);

    std::string guid = hscpp::CreateGuid();

    while (true)
    {
        swapper.Update();
    }

    return 0;
}