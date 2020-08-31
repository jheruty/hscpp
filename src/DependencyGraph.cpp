#include <algorithm>
#include <assert.h>

#include "hscpp/DependencyGraph.h"
#include "hscpp/Util.h"

namespace hscpp
{



    std::vector<hscpp::fs::path> DependencyGraph::ResolveGraph(const fs::path& filePath)
    {
        std::unordered_set<fs::path, FsPathHasher> collectedDependencyFilePaths;
        std::unordered_set<fs::path, FsPathHasher> collectedDependentFilePaths;

        // When compiling a module, add dependents of that module must also be compiled.
        if (IsModule(filePath))
        {
            CollectDependents(filePath, collectedDependentFilePaths);
        }

        // We want to compile all dependencies that are modules. If we have any dependents, their
        // dependencies must also be added to the compilation list.
        CollectDependencies(filePath, collectedDependencyFilePaths);
        for (const auto& collectedFilePath : collectedDependentFilePaths)
        {
            CollectDependencies(collectedFilePath, collectedDependencyFilePaths);
        }

        std::unordered_set<fs::path, FsPathHasher> collectedFilePaths;
        collectedFilePaths.insert(collectedDependencyFilePaths.begin(), collectedDependencyFilePaths.end());
        collectedFilePaths.insert(collectedDependentFilePaths.begin(), collectedDependentFilePaths.end());
        
        std::vector<fs::path> resolvedFilePaths;
        for (const auto& collectedFilePath : collectedFilePaths)
        {
            if (util::IsSourceFile(collectedFilePath))
            {
                resolvedFilePaths.push_back(collectedFilePath);
            }
        }
            
        return resolvedFilePaths;
    }

    void DependencyGraph::LinkFileToModule(const fs::path& filePath, const std::string& module)
    {
        m_FilePathsByModule[module].insert(filePath);
        m_ModulesByFilePath[filePath].insert(module);
    }

    void DependencyGraph::SetFileDependencies(const fs::path& filePath, const std::vector<fs::path>& dependencies)
    {
        int fileHandle = GetHandle(filePath);

        Node* pNode = GetNode(filePath);
        if (pNode == nullptr)
        {
            pNode = CreateNode(filePath);
        }

        // Remove reference to self from old dependencies.
        for (int dependencyHandle : pNode->dependencyHandles)
        {
            Node* pDependency = GetNode(dependencyHandle);
            pDependency->dependentHandles.erase(dependencyHandle);
        }

        // Add reference to self to new dependencies.
        pNode->dependencyHandles = AsHandleSet(dependencies);
        for (int dependencyHandle : pNode->dependencyHandles)
        {
            Node* pDependency = GetNode(dependencyHandle);
            if (pDependency == nullptr)
            {
                pDependency = CreateNode(dependencyHandle);
            }

            pDependency->dependentHandles.insert(fileHandle);
        }
    }

    void DependencyGraph::Clear()
    {
        m_FilePathsByModule.clear();
        m_ModulesByFilePath.clear();
        m_FilePathByHandle.clear();
        m_HandleByFilePath.clear();
        m_NodeByHandle.clear();
    }

    void DependencyGraph::Collect(const fs::path& filePath,
        std::unordered_set<fs::path, FsPathHasher>& collectedFilePaths, std::function<void(Node*)> cb)
    {
        if (collectedFilePaths.find(filePath) != collectedFilePaths.end())
        {
            return;
        }

        std::vector<fs::path> linkedFilePaths;

        bool bModule = IsModule(filePath);
        if (bModule)
        {
            linkedFilePaths = GetLinkedModuleFiles(filePath);
        }
        else
        {
            linkedFilePaths = { filePath };
        }

        for (const auto& linkedFilePath : linkedFilePaths)
        {
            if (collectedFilePaths.find(linkedFilePath) == collectedFilePaths.end())
            {
                collectedFilePaths.insert(linkedFilePath);
                Node* pNode = GetNode(linkedFilePath);
                if (pNode != nullptr)
                {
                    cb(pNode);
                }
            }
        }
    }

    void DependencyGraph::CollectDependencies(const fs::path& filePath,
        std::unordered_set<fs::path, FsPathHasher>& collectedFilePaths)
    {
        Collect(filePath, collectedFilePaths, [&](Node* pNode) {
            for (int dependencyHandle : pNode->dependencyHandles)
            {
                fs::path dependencyPath = GetFilepath(dependencyHandle);
                CollectDependencies(dependencyPath, collectedFilePaths);
            }
        });
    }

    void DependencyGraph::CollectDependents(const fs::path& filePath,
        std::unordered_set<fs::path, FsPathHasher>& collectedFilePaths)
    {
        Collect(filePath, collectedFilePaths, [&](Node* pNode) {
            for (int dependentHandle : pNode->dependentHandles)
            {
                fs::path dependentPath = GetFilepath(dependentHandle);
                CollectDependents(dependentPath, collectedFilePaths);
            }
        });
    }

    bool DependencyGraph::IsModule(const fs::path& filePath)
    {
        return m_ModulesByFilePath.find(filePath) != m_ModulesByFilePath.end();
    }

    std::vector<hscpp::fs::path> DependencyGraph::GetLinkedModuleFiles(const fs::path& filePath)
    {
        std::unordered_set<fs::path, FsPathHasher> linkedFilePaths;

        auto modulesIt = m_ModulesByFilePath.find(filePath);
        if (modulesIt != m_ModulesByFilePath.end())
        {
            for (const std::string& module : modulesIt->second)
            {
                auto linkedFilePathsIt = m_FilePathsByModule.find(module);
                if (linkedFilePathsIt != m_FilePathsByModule.end())
                {
                    for (const auto& linkedFilePath : linkedFilePathsIt->second)
                    {
                        linkedFilePaths.insert(linkedFilePath);
                    }
                }
            }
        }

        return std::vector<fs::path>(linkedFilePaths.begin(), linkedFilePaths.end());
    }

    int DependencyGraph::CreateHandle(const fs::path& filePath)
    {
        int handle = m_NextHandle;
        ++m_NextHandle;

        m_HandleByFilePath[filePath] = handle;
        m_FilePathByHandle[handle] = filePath;

        return handle;
    }

    int DependencyGraph::GetHandle(const fs::path& filePath)
    {
        auto it = m_HandleByFilePath.find(filePath);
        if (it != m_HandleByFilePath.end())
        {
            return it->second;
        }

        return CreateHandle(filePath);
    }

    hscpp::DependencyGraph::Node* DependencyGraph::CreateNode(const fs::path& filePath)
    {
        int handle = GetHandle(filePath);
        return CreateNode(handle);
    }

    hscpp::DependencyGraph::Node* DependencyGraph::CreateNode(int handle)
    {
        m_NodeByHandle[handle] = std::make_unique<Node>();

        Node* pNode = m_NodeByHandle[handle].get();
        return pNode;
    }

    hscpp::DependencyGraph::Node* DependencyGraph::GetNode(const fs::path& filePath)
    {
        int handle = GetHandle(filePath);
        return GetNode(handle);
    }

    hscpp::DependencyGraph::Node* DependencyGraph::GetNode(int handle)
    {
        auto it = m_NodeByHandle.find(handle);
        if (it != m_NodeByHandle.end())
        {
            return it->second.get();
        }

        return nullptr;
    }

    hscpp::fs::path DependencyGraph::GetFilepath(int handle)
    {
        auto it = m_FilePathByHandle.find(handle);
        if (it != m_FilePathByHandle.end())
        {
            return it->second;
        }

        return fs::path();
    }

    std::unordered_set<int> DependencyGraph::AsHandleSet(const std::vector<fs::path>& paths)
    {
        std::unordered_set<int> handleSet;
        for (const auto& path : paths)
        {
            handleSet.insert(GetHandle(path));
        }

        return handleSet;
    }

}