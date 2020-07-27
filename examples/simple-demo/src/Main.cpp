#include <iostream>
#include <thread>
#include <chrono>

#include "hscpp/Hotswapper.h"
#include "hscpp/FileWatcher.h"
#include "hscpp/Constructors.h"
#include "hscpp/StringUtil.h"

#include "Printer1.h"
#include "Printer2.h"
#include "hscpp/ModuleInterface.h"
#include "Ref.h"
#include "Memory.h"
#include "Allocator.h"

int main() 
{
    auto allocator = std::make_unique<Allocator>();
    hscpp::Hotswapper swapper(std::move(allocator));

    swapper.AddIncludeDirectory("./include");
    swapper.AddIncludeDirectory("../../include");
    swapper.AddSourceDirectory("./src", true);

    Ref<Printer1> p = TAllocator::Allocate<Printer1>();

    while (true)
    {
        swapper.Update();
        p->Update();
    }

    return 0;
}