#include <iostream>
#include <thread>
#include <conio.h>

#include "hscpp/Hotswapper.h"
#include "hscpp/AllocationResolver.h"
#include "hscpp-example-utils/Ref.h"
#include "hscpp-example-utils/MemoryManager.h"

#include "TrackedPrinter.h"


int main()
{
    hscpp::Hotswapper swapper;

    swapper.AddIncludeDirectory(std::filesystem::current_path());
    swapper.AddSourceDirectory(std::filesystem::current_path(), true);

    // After recompiling, the newly compiled module is capable of constructing new versions of of an
    // object. However, only hscpp knows about this new constructor. The MemoryManager is given a
    // reference to the hscpp::HotSwapper, so that it can use its Allocate<T>() method and choose the
    // most up-to-date constructor.
    MemoryManager::Instance().SetHotswapper(&swapper);

    // You can optionally set a memory allocator. If provided, hscpp will call Allocate, Reallocate
    // and Free, as per the hscpp::IAllocator interface. If no allocator is provided, hscpp uses
    // the standard 'new' and 'delete'.
    swapper.SetAllocator(&MemoryManager::Instance());

    // In the simple-demo example, we rely on using global memory shared across all modules in order
    // to handle when hscpp deletes an old object and creates a new one. Instead we can use the 'Ref'
    // concept, in which the Ref stores an unchanging id that refers to that object, regardless of the
    // object's memory location. This allows an object to change location in memory without references
    // to the object breaking.
    //
    // For Refs to work, a custom allocator must be provided, so that hscpp knows the object's id.
    Ref<TrackedPrinter> printer1 = MemoryManager::Allocate<TrackedPrinter>();
    Ref<TrackedPrinter> printer2 = MemoryManager::Allocate<TrackedPrinter>();

    printer1->Init(1);
    printer2->Init(2);

    auto then = std::chrono::system_clock::now();
    while (true)
    {
        swapper.Update();

        printer1->Update();
        printer2->Update();

        if (_kbhit())
        {
            int keycode = _getch();
            if (keycode == 'a')
            {
                // As a demo, make a change and and press 'a' to reconstruct a TrackedPrinter.
                // you should see a new Printer implementation is spawned. Then, stop the program,
                // and comment out this the code at the start of main:
                // 
                //      swapper.SetAllocator(&MemoryManager::Instance());
                // 
                // Now, TrackedPrinters that were already constructed will be correctly swapped out
                // with new objects using the updated implementation. However, newly constructed
                // TrackedPrinters will still be using the original implementation.
                //
                // For this reason, you'll want to use swapper.Allocate<T>() when allocating objects
                // with hscpp (see MemoryManager).
                MemoryManager::Allocate<TrackedPrinter>();
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}