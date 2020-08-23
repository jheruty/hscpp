#include "hscpp/DependencyGraph.h"

namespace hscpp
{

    hscpp::DependencyGraph::QueryResult DependencyGraph::ResolveGraph(const Query& query)
    {
        return QueryResult();
    }

    void DependencyGraph::LinkFileToModule(const fs::path& file, const std::string& module)
    {
        m_FilepathsByModule[module].insert(file.wstring());
        m_ModulesByFilepath[file.wstring()].insert(module);
    }

    void DependencyGraph::SetFileDependencies(const fs::path& file, const std::vector<fs::path>& dependencies)
    {

    }

}