#pragma once

#include <unordered_map>

#include "hscpp/IAllocator.h"

namespace hscpp
{
    class ITracker;

    class ModuleSharedState
    {
    public:
        inline static std::unordered_map<std::string, std::vector<ITracker*>>* s_pTrackersByKey = nullptr;
        inline static IAllocator* s_pAllocator = nullptr;
    };

}

