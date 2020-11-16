#include "catch/catch.hpp"
#include "common/Common.h"

#include "hscpp/preprocessor/DependencyGraph.h"

namespace hscpp { namespace test {

    TEST_CASE("DependencyGraph can handle minimal graph.")
    {
        DependencyGraph graph;

        fs::path childCpp = "child.cpp";
        fs::path parent1H = "parent1.h";
        fs::path parent1Cpp = "parent1.cpp";
        fs::path parent2H = "parent2.h";
        fs::path parent2Cpp = "parent2.cpp";

        graph.SetFileDependencies(childCpp, { parent1H, parent2H });
        graph.SetFileDependencies(parent1Cpp, { parent1H });
        graph.SetFileDependencies(parent2Cpp, { parent2H });

        graph.SetLinkedModules(parent1H, { "module" });
        graph.SetLinkedModules(parent2Cpp, { "module" });

        // child.cpp will see module in parent1.h, and needs to compile parent2.cpp.
        std::vector<fs::path> resolvedForChildCpp = graph.ResolveGraph(childCpp);
        CALL(ValidateUnorderedVector, resolvedForChildCpp, {
            childCpp,
            parent2Cpp,
        });

        // parent1.h is a module; all those who include it must be recompiled, as well as other
        // files within the module.
        std::vector<fs::path> resolvedForParent1H = graph.ResolveGraph(parent1H);
        CALL(ValidateUnorderedVector, resolvedForParent1H, {
            childCpp,
            parent1Cpp,
            parent2Cpp,
        });

        // parent2.h is not a module, nor a source file, nothing needs to be compiled.
        std::vector<fs::path> resolvedForParent2H = graph.ResolveGraph(parent2H);
        REQUIRE(resolvedForParent2H.empty());

        // parent2.cpp is a module, all dependents of that module must be compiled.
        std::vector<fs::path> resolvedForParent2Cpp = graph.ResolveGraph(parent2Cpp);
        CALL(ValidateUnorderedVector, resolvedForParent2Cpp, {
            childCpp,
            parent1Cpp,
            parent2Cpp,
        });

        // Remove file and recheck.
        graph.RemoveFile(parent2Cpp);

        resolvedForParent1H = graph.ResolveGraph(parent1H);
        CALL(ValidateUnorderedVector, resolvedForParent1H, {
            childCpp,
            parent1Cpp,
        });

        resolvedForChildCpp = graph.ResolveGraph(childCpp);
        CALL(ValidateUnorderedVector, resolvedForChildCpp, {
            childCpp,
        });
    }

    TEST_CASE("DependencyGraph can handle circular dependencies.")
    {
        DependencyGraph graph;

        fs::path pathA = "pathA.cpp";
        fs::path pathB = "pathB.cpp";

        graph.SetFileDependencies(pathA, { pathB });
        graph.SetFileDependencies(pathB, { pathA });

        graph.SetLinkedModules(pathA, { "module" });
        graph.SetLinkedModules(pathB, { "module" });

        std::vector<fs::path> resolved = graph.ResolveGraph(pathA);
        CALL(ValidateUnorderedVector, resolved, {
            pathA,
            pathB,
        });
    }

}}
