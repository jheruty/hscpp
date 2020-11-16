#pragma once

#include <vector>
#include <string>

#include "hscpp/Platform.h"
#include "hscpp/preprocessor/Variant.h"

namespace hscpp
{

    class IPreprocessor
    {
    public:
        struct Output
        {
            std::vector<fs::path> sourceFiles;
            std::vector<fs::path> includeDirectories;
            std::vector<fs::path> libraries;
            std::vector<fs::path> libraryDirectories;
            std::vector<std::string> preprocessorDefinitions;
        };

        virtual ~IPreprocessor() = default;

        virtual bool Preprocess(const std::vector<fs::path>& canonicalFilePaths, Output& output) = 0;

        virtual void SetVar(const std::string& name, const Variant& val) = 0;
        virtual bool RemoveVar(const std::string& name) = 0;

        virtual void ClearDependencyGraph() = 0;
        virtual void UpdateDependencyGraph(const std::vector<fs::path>& canonicalModifiedFiles,
                const std::vector<fs::path>& canonicalRemovedFiles,
                const std::vector<fs::path>& includeDirectories) = 0;
    };

}