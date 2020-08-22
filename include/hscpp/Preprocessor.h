#pragma once

#include <filesystem>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "hscpp/DependencyGraph.h"
#include "hscpp/FileParser.h"

namespace fs = std::filesystem;

namespace hscpp
{
    
    class Preprocessor
    {
    public:
        struct Input
        {
            std::vector<fs::path> files;
            std::vector<fs::path> includeDirectories;
            std::vector<fs::path> sourceDirectories;
            std::vector<fs::path> libraries;
            std::vector<std::string> preprocessorDefinitions;
            std::vector<std::string> cppHeaderExtensions;
            std::vector<std::string> cppSourceExtensions;

            std::unordered_map<std::string, std::string> hscppRequireVariables;
        };

        struct Output
        {
            std::vector<fs::path> files;
            std::vector<fs::path> includeDirectories;
            std::vector<fs::path> libraries;
            std::vector<std::string> preprocessorDefinitions;
        };

        Output Preprocess(const Input& input);

    private:
        FileParser m_FileParser;
        DependencyGraph m_DependencyGraph;

        void InterpolateRequireVariables(const Input& input, std::filesystem::path& path);
    };

}