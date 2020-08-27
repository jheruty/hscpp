#pragma once

#include <filesystem>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "hscpp/DependencyGraph.h"
#include "hscpp/FileParser.h"

namespace hscpp
{
    
    namespace fs = std::filesystem;

    class Preprocessor
    {
    public:
        struct Input
        {
            bool bHscppMacros = false;
            bool bDependentCompilation = false;

            std::vector<fs::path> sourceFiles;
            std::vector<fs::path> includeDirectories;
            std::vector<fs::path> sourceDirectories;
            std::vector<fs::path> libraries;
            std::vector<std::string> preprocessorDefinitions;
            std::unordered_map<std::string, std::string> hscppRequireVariables;
        };

        struct Output
        {
            std::vector<fs::path> files;
            std::vector<fs::path> includeDirectories;
            std::vector<fs::path> libraries;
            std::vector<std::string> preprocessorDefinitions;
        };

        void CreateDependencyGraph(const Input& input);
        Output Preprocess(const Input& input);

    private:
        FileParser m_FileParser;
        DependencyGraph m_DependencyGraph;

        std::unordered_set<std::wstring> m_SourceFiles;
        std::unordered_set<std::wstring> m_IncludeDirectories;
        std::unordered_set<std::wstring> m_Libraries;
        std::unordered_set<std::string> m_PreprocessorDefinitions;

        void Reset(const Input& input);
        Output CreateOutput();

        void AddRequires(const Input& input, const FileParser::ParseInfo& parseInfo);
        void AddPreprocessorDefinitions(const FileParser::ParseInfo& parseInfo);
        void UpdateDependencyGraph(const Input& input, const FileParser::ParseInfo& parseInfo);

        void InterpolateRequireVariables(const Input& input, fs::path& path);
    };

}