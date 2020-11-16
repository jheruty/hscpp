#include <iostream>
#include <thread>
#include <chrono>

#include "hscpp/Filesystem.h"
#include "hscpp/Hotswapper.h"
#include "hscpp/Util.h"
#include "hscpp-example-utils/MemoryManager.h"
#include "runtime-require-demo/Printer.h"

const static hscpp::fs::path DEMO_PATH = hscpp::util::GetHscppExamplesPath() / "runtime-require-demo";

int main()
{
    hscpp::Hotswapper swapper;

    auto srcPath = DEMO_PATH / "src";
    auto includePath = DEMO_PATH / "include";

    swapper.AddSourceDirectory(srcPath);
    swapper.AddIncludeDirectory(includePath);

    auto memoryManager = MemoryManager::Create(swapper.GetAllocationResolver());
    swapper.SetAllocator(&memoryManager);

    // Although we also depend on hscpp-example-utils, these are not added to the hscpp::Hotswapper,
    // to demonstrate the use of hscpp_require macros. See Printer.cpp for details.

    // The hscpp_require_ macros require enabling an optional feature.
    swapper.EnableFeature(hscpp::Feature::Preprocessor);

    // We can set variables that will be interpolated with ${VarName} in hscpp_ macros.
#ifdef _WIN32
    swapper.SetVar("os", "Windows");
#else
    swapper.SetVar("os", "Posix");
#endif

    swapper.SetVar("libDirectory", "../../../build/examples/hscpp-example-utils");

    swapper.AddPreprocessorDefinition("PREPROCESSOR_DEMO");

    Ref<Printer> printer = memoryManager->Allocate<Printer>();

    while (true)
    {
        swapper.Update();

        // You can skip updating while compiling, by checking the state of the hscpp::Hotswapper.
        if (!swapper.IsCompiling())
        {
            // In a protected call, a thrown exception will cause the hscpp::Hotswapper to wait
            // for code changes. Upon detecting code changes, the code will be recompiled and the
            // protected call will be reattempted.
            swapper.DoProtectedCall([&]() {
                printer->Update();
                });
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
