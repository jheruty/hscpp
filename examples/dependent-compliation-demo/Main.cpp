#include <thread>

#include "hscpp/Hotswapper.h"
#include "Printer1.h"
#include "Printer2.h"

int main()
{
    hscpp::Hotswapper swapper;

    swapper.AddIncludeDirectory(std::filesystem::current_path());
    swapper.AddSourceDirectory(std::filesystem::current_path());

    swapper.EnableFeature(hscpp::Feature::Preprocessor);
    swapper.EnableFeature(hscpp::Feature::DependentCompilation);

    swapper.CreateDependencyGraph();

    swapper.GetAllocationResolver()->Allocate<Printer1>();
    swapper.GetAllocationResolver()->Allocate<Printer2>();

    hscpp::Callbacks callbacks;
    callbacks.BeforeSwap = []() {std::cout << "Before swap callback." << std::endl; };
    callbacks.AfterSwap = []() {std::cout << "After swap callback." << std::endl; };
    callbacks.BeforePreprocessor = [](hscpp::Preprocessor::Input& input) {
        input.sourceFiles.push_back(std::filesystem::current_path() / "Printer2.cpp");
    };

    swapper.SetCallbacks(callbacks);
    swapper.EnableFeature(hscpp::Feature::ManualCompilationOnly);

    while (true)
    {
        swapper.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        swapper.TriggerManualBuild();
    }
}