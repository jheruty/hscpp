#include <algorithm>
#include <assert.h>

#include "hscpp/DependencyGraph.h"
#include "hscpp/Util.h"

namespace hscpp
{

    std::vector<fs::path> DependencyGraph::ResolveGraph(const fs::path& file)
    {
        std::vector<fs::path> files;

        std::unordered_set<std::wstring> filepaths;
        Collect(file, filepaths);

        std::unordered_set<std::wstring> visitedIncludeDirectories;
        for (const auto& filepath : filepaths)
        {
            fs::path file = filepath;

            if (util::IsSourceFile(file))
            {
                files.push_back(file);
            }
        }

        return files;
    }

    void DependencyGraph::LinkFileToModule(const fs::path& file, const std::string& module)
    {
        if (!FilePassesFilter(file))
        {
            return;
        }

        m_FilepathsByModule[module].insert(file.wstring());
        m_ModulesByFilepath[file.wstring()].insert(module);
    }

    void DependencyGraph::SetFileDependencies(const fs::path& file, const std::vector<fs::path>& dependencies)
    {
        if (!FilePassesFilter(file))
        {
            return;
        }

        std::vector<fs::path> filteredDependencies = FilterFiles(dependencies);

        Node* pNode = GetNode(file);
        if (pNode == nullptr)
        {
            pNode = CreateNode(file);
        }

        // Dependencies have been updated, remove reference to self from old dependencies.
        for (int dependencyHandle : pNode->dependencies)
        {
            Node* pDependency = GetNode(dependencyHandle);
            pDependency->dependents.erase(dependencyHandle);
        }

        // Add reference to self to dependencies.
        pNode->dependencies = AsHandleSet(filteredDependencies);
        for (int dependencyHandle : pNode->dependencies)
        {
            Node* pDependency = GetNode(dependencyHandle);
            if (pDependency == nullptr)
            {
                pDependency = CreateNode(dependencyHandle);
            }

            int handle = GetHandle(file);
            pDependency->dependents.insert(handle);
        }
    }

    void DependencyGraph::Clear()
    {
        m_FilepathsByModule.clear();
        m_ModulesByFilepath.clear();

        m_HandleByFilepath;
        m_FilepathByHandle.clear();

        m_NodeByHandle.clear();

        m_NextHandle = 0;
    }

    void DependencyGraph::Collect(const fs::path& file, std::unordered_set<std::wstring>& filepaths)
    {
        if (filepaths.find(file) != filepaths.end())
        {
            return;
        }

        std::vector<fs::path> files = GetLinkedModuleFiles(file);

        bool bCollectDependents = true;
        if (files.empty())
        {
            // TODO: Explain how this fixes issue with Main.cpp.
            bCollectDependents = false;
            files = { file };
        }

        for (const auto& linkedFile : files)
        {
            if (filepaths.find(linkedFile) == filepaths.end())
            {
                filepaths.insert(linkedFile);

                Node* pNode = GetNode(linkedFile);
                if (pNode != nullptr)
                {
                    for (int dependencyHandle : pNode->dependencies)
                    {
                        fs::path dependencyPath = GetFilepath(dependencyHandle);
                        Collect(dependencyPath, filepaths);
                    }

                    if (bCollectDependents)
                    {
                        for (int dependentHandle : pNode->dependents)
                        {
                            fs::path dependentPath = GetFilepath(dependentHandle);
                            Collect(dependentPath, filepaths);
                        }
                    }
                }
            }
        }
    }

    bool DependencyGraph::FilePassesFilter(const fs::path& file)
    {
        if (util::IsHeaderFile(file) || util::IsSourceFile(file))
        {
            return true;
        }

        return false;
    }

    std::vector<fs::path> DependencyGraph::FilterFiles(const std::vector<fs::path>& files)
    {
        std::vector<fs::path> filteredFiles;
        for (const auto& file : files)
        {
            if (FilePassesFilter(file))
            {
                filteredFiles.push_back(file);
            }
        }

        return filteredFiles;
    }

    int DependencyGraph::CreateHandle(const fs::path& path)
    {
        ++m_NextHandle;
        m_HandleByFilepath[path.wstring()] = m_NextHandle;
        m_FilepathByHandle[m_NextHandle] = path.wstring();

        return m_NextHandle;
    }

    int DependencyGraph::GetHandle(const fs::path& path)
    {
        auto it = m_HandleByFilepath.find(path.wstring());
        if (it != m_HandleByFilepath.end())
        {
            return it->second;
        }

        return CreateHandle(path);
    }

    hscpp::fs::path DependencyGraph::GetFilepath(int handle)
    {
        auto it = m_FilepathByHandle.find(handle);
        if (it != m_FilepathByHandle.end())
        {
            return it->second;
        }

        return fs::path();
    }

    hscpp::DependencyGraph::Node* DependencyGraph::CreateNode(const fs::path& file)
    {
        int handle = GetHandle(file);
        m_NodeByHandle[handle] = std::make_unique<Node>();

        Node* pNode = m_NodeByHandle[handle].get();
        return pNode;
    }

    hscpp::DependencyGraph::Node* DependencyGraph::CreateNode(int handle)
    {
        fs::path filepath = GetFilepath(handle);
        return CreateNode(filepath);
    }

    hscpp::DependencyGraph::Node* DependencyGraph::GetNode(const fs::path& file)
    {
        int handle = GetHandle(file);
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

    std::vector<fs::path> DependencyGraph::GetLinkedModuleFiles(const fs::path& file)
    {
        std::unordered_set<std::wstring> linkedFiles;

        auto modulesIt = m_ModulesByFilepath.find(file.wstring());
        if (modulesIt != m_ModulesByFilepath.end())
        {
            for (const std::string& module : modulesIt->second)
            {
                auto filepathsIt = m_FilepathsByModule.find(module);
                if (filepathsIt != m_FilepathsByModule.end())
                {
                    for (const std::wstring& filepath : filepathsIt->second)
                    {
                        linkedFiles.insert(filepath);
                    }
                }
            }
        }

        return std::vector<fs::path>(linkedFiles.begin(), linkedFiles.end());
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