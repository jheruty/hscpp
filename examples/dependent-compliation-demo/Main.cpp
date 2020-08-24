#include <thread>

#include "hscpp/Hotswapper.h"
#include "Printer1.h"
#include "Printer2.h"

int main()
{
    hscpp::Hotswapper swapper;

    swapper.AddIncludeDirectory(std::filesystem::current_path());
    swapper.AddSourceDirectory(std::filesystem::current_path());

    swapper.EnableFeature(hscpp::Feature::HscppMacros);
    swapper.EnableFeature(hscpp::Feature::DependentCompilation);

    swapper.CreateDependencyGraph();

    swapper.GetAllocationResolver()->Allocate<Printer1>();
    swapper.GetAllocationResolver()->Allocate<Printer2>();

    while (true)
    {
        swapper.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}