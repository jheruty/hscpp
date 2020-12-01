#include <thread>
#include <chrono>

#include "hscpp/Filesystem.h"
#include "hscpp/Hotswapper.h"
#include "hscpp/Util.h"
#include "simple-demo/SimpleDemoData.h"
#include "simple-demo/Printer.h"

const static hscpp::fs::path DEMO_PATH = hscpp::util::GetHscppExamplesPath() / "simple-demo";

int main()
{
    hscpp::Hotswapper swapper;

    auto srcPath = DEMO_PATH / "src";
    auto includePath = DEMO_PATH / "include";

    // Watch the source and include directories of this example for changes.
    swapper.AddSourceDirectory(srcPath);
    swapper.AddIncludeDirectory(includePath);

    // When an object is recompiled, a new dynamic library is linked into the running program
    // with its own statics and globals. You can give a pointer to user-defined data which will
    // be shared across all modules, since normal statics and globals will not be visible in
    // the newly compiled library.
    SimpleDemoData data;
    swapper.SetGlobalUserData(&data);

    // Create a couple of new Printers, and let them know their index.
    data.printers.at(0) = swapper.GetAllocationResolver()->Allocate<Printer>();
    data.printers.at(1) = swapper.GetAllocationResolver()->Allocate<Printer>();

    data.printers.at(0)->Init("PrinterA", 0);
    data.printers.at(1)->Init("PrinterB", 1);

    while (true)
    {
        // Update the Hotswapper, which will load a new DLL on changes.
        swapper.Update();
        
        data.printers.at(0)->Update();
        data.printers.at(1)->Update();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
