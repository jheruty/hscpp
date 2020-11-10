#pragma once

#include <string>
#include <unordered_map>

#include "hscpp/preprocessor/Variant.h"

namespace hscpp
{

    class VarStore
    {
    public:
        void SetVar(const std::string& name, const Variant& val);

        bool GetVar(const std::string& name, Variant& val) const;
        bool RemoveVar(const std::string& name);

        std::string Interpolate(const std::string& str) const;

    private:
        std::unordered_map<std::string, Variant> m_Vars;
    };

}