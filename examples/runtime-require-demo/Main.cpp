#include <iostream>
#include <thread>
#include <chrono>

#include "hscpp/Hotswapper.h"
#include "hscpp-example-utils/MemoryManager.h"
#include "Printer.h"

int main()
{
    hscpp::Hotswapper swapper;

    swapper.AddIncludeDirectory(std::filesystem::current_path());
    swapper.AddSourceDirectory(std::filesystem::current_path(), true);

    auto memoryManager = MemoryManager::Create(swapper.GetAllocationResolver());
    swapper.SetAllocator(&memoryManager);

    // Although we also depend on hscpp-example-utils, these are not added to the hscpp::Hotswapper,
    // to demonstrate the use of hscpp_require macros. See Printer.cpp for details.

    // The hscpp_require_ macros require enabling an optional feature.
    swapper.EnableFeature(hscpp::Feature::HscppRequire);

    // We can set variables that will be interpolated with %VAR_NAME% in hscpp_require_ macros. Note
    // that these will take precedence over environment variables of the same name.
#ifdef _DEBUG
    swapper.SetHscppRequireVariable("PROJECT_CONFIGURATION", "Debug");
#else
    swapper.SetHscppRequireVariable("PROJECT_CONFIGURATION", "Release");
#endif

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