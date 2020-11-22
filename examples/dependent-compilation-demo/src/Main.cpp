#include <thread>

#include "hscpp/Filesystem.h"
#include "hscpp/Hotswapper.h"
#include "hscpp/Util.h"
#include "dependent-compilation-demo/Printer1.h"
#include "dependent-compilation-demo/Printer2.h"

const static hscpp::fs::path DEMO_PATH = hscpp::util::GetHscppExamplesPath() / "dependent-compilation-demo";

int main()
{
    hscpp::Hotswapper swapper;

    auto srcPath = DEMO_PATH / "src";
    auto includePath = DEMO_PATH / "include";

    swapper.AddSourceDirectory(srcPath);
    swapper.AddIncludeDirectory(includePath);

    // Add include directory as a source directory, so that the dependency graph will be able to
    // link hscpp_modules across source and include directories.
    swapper.AddSourceDirectory(includePath / "dependent-compilation-demo");

    // Must enable hscpp::DependentCompilation feature. This also implicitly enables the
    // hscpp::Preprocessor feature.
    swapper.EnableFeature(hscpp::Feature::DependentCompilation);

    swapper.GetAllocationResolver()->Allocate<Printer1>();
    swapper.GetAllocationResolver()->Allocate<Printer2>();

    // To create a dependency graph, hscpp will parse all files in the directories provided within
    // the hscpp::Hotswapper's AddIncludeDirectory and AddSourceDirectory functions. #includes in
    // those files will be parsed to generate the graph.
    //
    // When a file in the dependency graph is saved, it will traverse the dependency graph to
    // determine what other files would need to be compiled, and adds them to the build.

    while (true)
    {
        swapper.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
