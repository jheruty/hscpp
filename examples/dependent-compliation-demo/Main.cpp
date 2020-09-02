#include <thread>

#include "hscpp/Hotswapper.h"
#include "Printer1.h"
#include "Printer2.h"

int main()
{
    hscpp::Hotswapper swapper;

    swapper.AddIncludeDirectory(std::filesystem::current_path());
    swapper.AddSourceDirectory(std::filesystem::current_path());

    // Must enable hscpp::DependentCompilation feature. This also implicitly enables the
    // hscpp::Preprocessor feature.
    swapper.EnableFeature(hscpp::Feature::DependentCompilation);

    // To create a dependency graph, hscpp will parse all files in the directories provided within
    // the hscpp::Hotswapper's AddIncludeDirectory and AddSourceDirectory functions. #includes in
    // those files will be parsed to generate the graph.
    swapper.CreateDependencyGraph();

    swapper.GetAllocationResolver()->Allocate<Printer1>();
    swapper.GetAllocationResolver()->Allocate<Printer2>();

    while (true)
    {
        swapper.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}