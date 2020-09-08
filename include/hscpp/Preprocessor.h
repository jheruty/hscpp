#pragma once

#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "hscpp/Platform.h"
#include "hscpp/DependencyGraph.h"
#include "hscpp/FileParser.h"
#include "hscpp/FsPathHasher.h"
#include "hscpp/FeatureManager.h"

namespace hscpp
{
    
    class Preprocessor
    {
    public:
        struct Input
        {
            std::vector<fs::path> sourceFilePaths;
            std::vector<fs::path> includeDirectoryPaths;
            std::vector<fs::path> sourceDirectoryPaths;
            std::vector<fs::path> libraryPaths;
            std::vector<std::string> preprocessorDefinitions;
            std::unordered_map<std::string, std::string> hscppRequireVariables;
        };

        struct Output
        {
            std::vector<fs::path> sourceFilePaths;
            std::vector<fs::path> includeDirectoryPaths;
            std::vector<fs::path> libraryPaths;
            std::vector<std::string> preprocessorDefinitions;
        };

        void SetFeatureManager(FeatureManager* pFeatureManager);

        void CreateDependencyGraph(const Input& input);
        void PruneDeletedFilesFromDependencyGraph();
        Output Preprocess(const Input& input);

    private:
        FileParser m_FileParser;
        DependencyGraph m_DependencyGraph;

        std::unordered_set<fs::path, FsPathHasher> m_SourceFiles;
        std::unordered_set<fs::path, FsPathHasher> m_IncludeDirectories;
        std::unordered_set<fs::path, FsPathHasher> m_Libraries;
        std::unordered_set<std::string> m_PreprocessorDefinitions;

        FeatureManager* m_pFeatureManager = nullptr;

        void Reset(const Input& input);
        Output CreateOutput();

        void AddRequires(const Input& input, const FileParser::ParseInfo& parseInfo);
        void AddPreprocessorDefinitions(const FileParser::ParseInfo& parseInfo);
        void UpdateDependencyGraph(const Input& input, const FileParser::ParseInfo& parseInfo);

        void InterpolateRequireVariables(const Input& input, fs::path& path);
    };

}