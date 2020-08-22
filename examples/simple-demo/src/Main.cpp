#include <thread>
#include <chrono>
#include <filesystem>

#include "hscpp/Hotswapper.h"
#include "SimpleDemoData.h"
#include "Printer.h"

int main()
{
    hscpp::Hotswapper hotswapper;

    // Watch the current directory for changes.
    hotswapper.AddIncludeDirectory(std::filesystem::current_path());
    hotswapper.AddSourceDirectory(std::filesystem::current_path());

    // When an object is recompiled, a new DLL is linked into the running program with its own
    // statics and globals. You can give a pointer to user-defined data which will be shared
    // across all modules, since normal statics and globals will be lost.
    SimpleDemoData data;
    hotswapper.SetGlobalUserData(&data);

    // Create a couple new Printers, and let them know their index into pInstances.
    data.pInstances[0] = new Printer("PrinterA", 0);
    data.pInstances[1] = new Printer("PrinterB", 1);

    while (true)
    {
        // Update the Hotswapper, which will load a new DLL on changes.
        hotswapper.Update();

        data.pInstances[0]->Update();
        data.pInstances[1]->Update();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}