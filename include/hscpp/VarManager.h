#pragma once

#include <string>
#include <unordered_map>

namespace hscpp
{

    class VarManager
    {
    public:
        void SetVar(const std::string& name, const std::string& val);
        bool RemoveVar(const std::string& name);

        std::string Interpolate(const std::string& str);

    private:
        std::unordered_map<std::string, std::string> m_Vars;
    };

}