#include <thread>

#include "hscpp/Hotswapper.h"
#include "dependent-compilation-demo/Printer1.h"
#include "dependent-compilation-demo/Printer2.h"

int main()
{
    hscpp::Hotswapper swapper;

    auto srcPath = std::filesystem::path(__FILE__).parent_path();
    auto includePath = srcPath.parent_path() / "include";

    swapper.AddSourceDirectory(srcPath);
    swapper.AddIncludeDirectory(includePath);

    // Add include directory as a source directory, so that the dependency graph will be able to
    // link hscpp_modules across source and include directories.
    swapper.AddSourceDirectory(includePath / "dependent-compilation-demo");

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
