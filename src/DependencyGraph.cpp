#include <algorithm>
#include <assert.h>

#include "hscpp/DependencyGraph.h"
#include "hscpp/Util.h"

namespace hscpp
{



    std::vector<hscpp::fs::path> DependencyGraph::ResolveGraph(const fs::path& filePath)
    {
        std::unordered_set<int> collectedDependentHandles;
        std::unordered_set<int> collectedDependencyHandles;

        int fileHandle = GetHandle(filePath);

        // When compiling a module, add dependents of that module must also be compiled.
        if (IsModule(fileHandle))
        {
            CollectDependents(fileHandle, collectedDependentHandles);
        }

        // We want to compile all dependencies that are modules. If we have any dependents, their
        // dependencies must also be added to the compilation list.
        CollectDependencies(fileHandle, collectedDependencyHandles);
        for (int collectedDependentHandle : collectedDependentHandles)
        {
            CollectDependencies(collectedDependentHandle, collectedDependencyHandles);
        }

        std::unordered_set<int> collectedHandles;
        collectedHandles.insert(collectedDependentHandles.begin(), collectedDependentHandles.end());
        collectedHandles.insert(collectedDependencyHandles.begin(), collectedDependencyHandles.end());
        
        std::vector<fs::path> resolvedFilePaths;
        for (const auto& collectedHandle : collectedHandles)
        {
            fs::path collectedFilePath = GetFilepath(collectedHandle);
            if (util::IsSourceFile(collectedFilePath))
            {
                resolvedFilePaths.push_back(collectedFilePath);
            }
        }
            
        return resolvedFilePaths;
    }

    void DependencyGraph::LinkFileToModule(const fs::path& filePath, const std::string& module)
    {
        int handle = GetHandle(filePath);

        m_HandlesByModule[module].insert(handle);
        m_ModulesByHandle[handle].insert(module);
    }

    void DependencyGraph::SetFileDependencies(const fs::path& filePath, const std::vector<fs::path>& dependencies)
    {
        int fileHandle = GetHandle(filePath);

        Node* pNode = GetNode(fileHandle);
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
        m_HandlesByModule.clear();
        m_ModulesByHandle.clear();
        m_FilePathByHandle.clear();
        m_HandleByFilePath.clear();
        m_NodeByHandle.clear();
    }

    void DependencyGraph::Collect(int handle, std::unordered_set<int>& collectedHandles, std::function<void(Node*)> cb)
    {
        if (collectedHandles.find(handle) != collectedHandles.end())
        {
            return;
        }

        std::vector<int> linkedHandles;

        bool bModule = IsModule(handle);
        if (bModule)
        {
            linkedHandles = GetLinkedModuleHandles(handle);
        }
        else
        {
            linkedHandles = { handle };
        }

        for (const int linkedHandle : linkedHandles)
        {
            if (collectedHandles.find(linkedHandle) == collectedHandles.end())
            {
                collectedHandles.insert(linkedHandle);
                Node* pNode = GetNode(linkedHandle);
                if (pNode != nullptr)
                {
                    cb(pNode);
                }
            }
        }
    }

    void DependencyGraph::CollectDependencies(int handle, std::unordered_set<int>& collectedHandles)
    {
        Collect(handle, collectedHandles, [&](Node* pNode) {
            for (int dependencyHandle : pNode->dependencyHandles)
            {
                CollectDependencies(dependencyHandle, collectedHandles);
            }
        });
    }

    void DependencyGraph::CollectDependents(int handle, std::unordered_set<int>& collectedHandles)
    {
        Collect(handle, collectedHandles, [&](Node* pNode) {
            for (int dependentHandle : pNode->dependentHandles)
            {
                CollectDependents(dependentHandle, collectedHandles);
            }
        });
    }

    bool DependencyGraph::IsModule(int handle)
    {
        auto it = m_ModulesByHandle.find(handle);
        if (it != m_ModulesByHandle.end())
        {
            return !m_ModulesByHandle.empty();
        }

        return false;
    }

    std::vector<int> DependencyGraph::GetLinkedModuleHandles(int handle)
    {
        std::unordered_set<int> linkedHandles;

        auto modulesIt = m_ModulesByHandle.find(handle);
        if (modulesIt != m_ModulesByHandle.end())
        {
            for (const std::string& module : modulesIt->second)
            {
                auto linkedHandlesIt = m_HandlesByModule.find(module);
                if (linkedHandlesIt != m_HandlesByModule.end())
                {
                    for (const auto& linkedHandle : linkedHandlesIt->second)
                    {
                        linkedHandles.insert(linkedHandle);
                    }
                }
            }
        }

        return std::vector<int>(linkedHandles.begin(), linkedHandles.end());
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