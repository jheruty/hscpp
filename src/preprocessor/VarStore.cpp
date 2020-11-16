#include "hscpp/preprocessor/VarStore.h"
#include "hscpp/Log.h"
#include "hscpp/Util.h"

namespace hscpp
{

    void VarStore::SetVar(const std::string& name, const Variant& val)
    {
        m_Vars[util::Trim(name)] = val;
    }

    bool VarStore::GetVar(const std::string& name, Variant& val) const
    {
        auto varIt = m_Vars.find(util::Trim(name));
        if (varIt ==m_Vars.end())
        {
            return false;
        }

        val = varIt->second;
        return true;
    }

    bool VarStore::RemoveVar(const std::string& name)
    {
        auto varIt = m_Vars.find(util::Trim(name));
        if (varIt == m_Vars.end())
        {
            return false;
        }

        m_Vars.erase(varIt);
        return true;
    }

    std::string VarStore::Interpolate(const std::string& str) const
    {
        std::string interpolatedStr = str;

        // Find and replace, for example "The ${ VarName }" with "The VarValue".
        std::string startPattern = "${";
        std::string endPattern = "}";

        size_t iStart = interpolatedStr.find(startPattern);
        while (iStart != std::string::npos)
        {
            size_t iEnd = interpolatedStr.find(endPattern, iStart + 1);
            if (iEnd != std::string::npos)
            {
                size_t iMatchStart = iStart + startPattern.size();
                size_t iMatchEnd = iEnd - endPattern.size();

                std::string varName = interpolatedStr.substr(iMatchStart, iMatchEnd - iMatchStart + 1);
                varName = util::Trim(varName);

                auto varIt = m_Vars.find(varName);
                if (varIt != m_Vars.end())
                {
                    interpolatedStr.replace(iStart, iEnd - iStart + 1, varIt->second.ToString());
                }
            }

            // String may have been lengthened or shortened, so continue search from the last start
            // since that index is stable.
            iStart = interpolatedStr.find(startPattern, iStart + 1);
        }

        return interpolatedStr;
    }

}