#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "hscpp/Platform.h"
#include "hscpp/FsPathHasher.h"

namespace hscpp
{

    class DependencyGraph
    {
    public:
        std::vector<fs::path> ResolveGraph(const fs::path& filePath);

        void SetLinkedModules(const fs::path& filePath, const std::vector<std::string>& modules);
        void SetFileDependencies(const fs::path& filePath, const std::vector<fs::path>& dependencies);
        void RemoveFile(const fs::path& filePath);

        void Clear();
        
    private:
        // Map integers to filepaths to avoid storing a large number of duplicated paths, and to
        // increase the speed of lookups.
        struct Node
        {
            std::unordered_set<int> dependencyHandles;
            std::unordered_set<int> dependentHandles;
        };

        std::unordered_map<std::string, std::unordered_set<int>> m_HandlesByModule;
        std::unordered_map<int, std::unordered_set<std::string>> m_ModulesByHandle;

        std::unordered_map<int, fs::path> m_FilePathByHandle;
        std::unordered_map<fs::path, int, FsPathHasher> m_HandleByFilePath;

        std::unordered_map<int, std::unique_ptr<Node>> m_NodeByHandle;

        int m_NextHandle = 0;

        void Collect(int handle, std::unordered_set<int>& collectedHandles, const std::function<void(Node*)>& cb);
        void CollectDependencies(int handle, std::unordered_set<int>& collectedHandles);
        void CollectDependents(int handle, std::unordered_set<int>& collectedHandles);

        bool IsModule(int handle);
        std::vector<int> GetLinkedModuleHandles(int handle);
        void RemoveLinkedModule(int handle);

        int CreateHandle(const fs::path& filePath);
        int GetHandle(const fs::path& filePath);

        Node* CreateNode(const fs::path& filePath);
        Node* CreateNode(int handle);
        Node* GetNode(int handle);

        fs::path GetFilepath(int handle);

        std::unordered_set<int> AsHandleSet(const std::vector<fs::path>& paths);

    };

}