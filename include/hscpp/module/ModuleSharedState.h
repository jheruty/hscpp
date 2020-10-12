#pragma once

#include <unordered_map>
#include <string>

#include "hscpp/module/IAllocator.h"

namespace hscpp
{

    class ITracker;
    class IConstructor;

    class ModuleSharedState
    {
    public:
        // Internal global state required by hscpp. Modify at your own peril.
        static bool* s_pbSwapping;
        static std::unordered_map<std::string, std::vector<ITracker*>>* s_pTrackersByKey;
        static std::unordered_map<std::string, IConstructor*>* s_pConstructorsByKey;
        static IAllocator* s_pAllocator;
    };

}

