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
        std::vector<fs::path> ResolveGraph(const fs::path& file);

        void LinkFileToModule(const fs::path& file, const std::string& module);
        void SetFileDependencies(const fs::path& file, const std::vector<fs::path>& dependencies);

        void Clear();

    private:
        struct Node 
        {
            std::unordered_set<int> dependencies;
            std::unordered_set<int> dependents;
        };

        std::unordered_map<std::string, std::unordered_set<std::wstring>> m_FilepathsByModule;
        std::unordered_map<std::wstring, std::unordered_set<std::string>> m_ModulesByFilepath;

        std::unordered_map<std::wstring, int> m_HandleByFilepath;
        std::unordered_map<int, std::wstring> m_FilepathByHandle;

        std::unordered_map<int, std::unique_ptr<Node>> m_NodeByHandle;

        int m_NextHandle = 0;

        void Collect(const fs::path& file, std::unordered_set<std::wstring>& filepaths);

        bool FilePassesFilter(const fs::path& file);
        std::vector<fs::path> FilterFiles(const std::vector<fs::path>& files);

        int CreateHandle(const fs::path& path);

        int GetHandle(const fs::path& path);
        fs::path GetFilepath(int handle);

        Node* CreateNode(const fs::path& file);
        Node* CreateNode(int handle);

        Node* GetNode(const fs::path& file);
        Node* GetNode(int handle);

        std::vector<fs::path> GetLinkedModuleFiles(const fs::path& file);

        std::unordered_set<int> AsHandleSet(const std::vector<fs::path>& paths);
    };

}