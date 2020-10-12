#include "hscpp/VarManager.h"
#include "hscpp/Log.h"

namespace hscpp
{

    void VarManager::SetVar(const std::string& name, const std::string& val)
    {
        m_Vars[name] = val;
    }

    bool VarManager::RemoveVar(const std::string& name)
    {
        auto varIt = m_Vars.find(name);
        if (varIt == m_Vars.end())
        {
            return false;
        }

        m_Vars.erase(varIt);
        return true;
    }

    std::string VarManager::Interpolate(const std::string& str)
    {
        std::string interpolatedStr = str;
        for (const auto& var : m_Vars)
        {
            std::string search = "${" + var.first + "}";

            size_t iFound = interpolatedStr.find(search);
            while (iFound != std::string::npos)
            {
                interpolatedStr.replace(iFound, search.size(), var.second);
                iFound = interpolatedStr.find(search);
            }
        }

        return interpolatedStr;
    }

}