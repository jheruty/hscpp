#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace hscpp
{

    namespace fs = std::filesystem;

    class DependencyGraph
    {
    public:
        struct Query
        {
            fs::path file;
            std::vector<std::string> cppHeaderExtensions;
            std::vector<std::string> cppSourceExtensions;
        };

        struct QueryResult
        {
            std::vector<fs::path> includeDirectories;
            std::vector<fs::path> sourceFiles;
        };

        QueryResult ResolveGraph(const Query& query);

        void LinkFileToModule(const fs::path& file, const std::string& module);
        void SetFileDependencies(const fs::path& file, const std::vector<fs::path>& dependencies);

    private:
        enum class FileType
        {
            Header,
            Source,
        };

        struct Node 
        {
            fs::path file;
            FileType type = {};

            std::unordered_set<Node*> dependencies;
            std::unordered_set<Node*> dependents;
        };

        std::unordered_map<std::string, std::unordered_set<std::wstring>> m_FilepathsByModule;
        std::unordered_map<std::wstring, std::unordered_set<std::string>> m_ModulesByFilepath;
        std::unordered_map<std::wstring, std::unique_ptr<Node>> m_NodeByFilepath;
    };

}