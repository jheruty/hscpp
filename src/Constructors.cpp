#pragma once

#include "hscpp/Constructors.h"

namespace hscpp
{

    hscpp::ISwappable* Constructors::Create(const std::string& key)
    {
        auto constructorIt = GetConstructorsByKey().find(key);
        if (constructorIt != GetConstructorsByKey().end()) {
            IConstructor* constructor = GetConstructors().at(constructorIt->second).get();
            return constructor->Construct();
        }

        return nullptr;
    }

    std::vector<std::unique_ptr<hscpp::IConstructor>>& Constructors::GetConstructors()
    {
        static std::vector<std::unique_ptr<hscpp::IConstructor>> constructors;
        return constructors;
    }

    std::unordered_map<std::string, size_t>& Constructors::GetConstructorsByKey()
    {
        static std::unordered_map<std::string, size_t> iConstructorByKey;
        return iConstructorByKey;
    }

}