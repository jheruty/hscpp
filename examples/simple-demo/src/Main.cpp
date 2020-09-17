#include <thread>
#include <chrono>

#include "hscpp/Filesystem.h"
#include "hscpp/Hotswapper.h"
#include "simple-demo/SimpleDemoData.h"
#include "simple-demo/Printer.h"

int main()
{
    hscpp::Hotswapper swapper;

    auto srcPath = hscpp::fs::path(__FILE__).parent_path();
    auto includePath = srcPath.parent_path() / "include";

    // Watch the current directory for changes.
    swapper.AddSourceDirectory(srcPath);
    swapper.AddIncludeDirectory(includePath);

    // When an object is recompiled, a new DLL is linked into the running program with its own
    // statics and globals. You can give a pointer to user-defined data which will be shared
    // across all modules, since normal statics and globals will be lost.
    SimpleDemoData data;
    swapper.SetGlobalUserData(&data);

    // Create a couple new Printers, and let them know their index into pInstances.
    data.pInstances[0] = new Printer("PrinterA", 0);
    data.pInstances[1] = new Printer("PrinterB", 1);

    while (true)
    {
        // Update the Hotswapper, which will load a new DLL on changes.
        swapper.Update();

        data.pInstances[0]->Update();
        data.pInstances[1]->Update();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
