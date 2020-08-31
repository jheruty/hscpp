#pragma once

#include <filesystem>
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

        void LinkFileToModule(const fs::path& filePath, const std::string& module);
        void SetFileDependencies(const fs::path& filePath, const std::vector<fs::path>& dependencies);

        void Clear();
        
    private:
        // Map integers to filepaths to avoid storing a large number of duplicated paths, and to
        // increase the speed of lookups.
        struct Node
        {
            std::unordered_set<int> dependencyHandles;
            std::unordered_set<int> dependentHandles;
        };

        std::unordered_map<std::string, std::unordered_set<fs::path, FsPathHasher>> m_FilePathsByModule;
        std::unordered_map<fs::path, std::unordered_set<std::string>, FsPathHasher> m_ModulesByFilePath;

        std::unordered_map<int, fs::path> m_FilePathByHandle;
        std::unordered_map<fs::path, int, FsPathHasher> m_HandleByFilePath;

        std::unordered_map<int, std::unique_ptr<Node>> m_NodeByHandle;

        int m_NextHandle = 0;

        void Collect(const fs::path& filePath,
            std::unordered_set<fs::path, FsPathHasher>& collectedFilePaths, std::function<void(Node*)> cb);
        void CollectDependencies(const fs::path& filePath,
            std::unordered_set<fs::path, FsPathHasher>& collectedFilePaths);
        void CollectDependents(const fs::path& filePath,
            std::unordered_set<fs::path, FsPathHasher>& collectedFilePaths);

        bool IsModule(const fs::path& filePath);
        std::vector<fs::path> GetLinkedModuleFiles(const fs::path& filePath);

        int CreateHandle(const fs::path& filePath);
        int GetHandle(const fs::path& filePath);

        Node* CreateNode(const fs::path& filePath);
        Node* CreateNode(int handle);
        Node* GetNode(const fs::path& filePath);
        Node* GetNode(int handle);

        fs::path GetFilepath(int handle);

        std::unordered_set<int> AsHandleSet(const std::vector<fs::path>& paths);

    };

}