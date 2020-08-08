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
        inline static bool* s_pbSwapping = nullptr;
        inline static std::unordered_map<std::string, std::vector<ITracker*>>* s_pTrackersByKey = nullptr;
        inline static std::unordered_map<std::string, IConstructor*>* s_pConstructorsByKey = nullptr;
        inline static IAllocator* s_pAllocator = nullptr;
    };

}

