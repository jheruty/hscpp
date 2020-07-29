#pragma once

#include <unordered_map>

#include "hscpp/IAllocator.h"

namespace hscpp
{
    class ITracker;

    class ModuleSharedState
    {
    public:
        // Users can place their own data here to be used across all modules.
        inline static void* s_pGlobalUserData = nullptr;

    private:
        friend class ModuleInterface;

        template <typename T, const char* Key>
        friend class Tracker;

        template <typename T>
        friend class Constructor;

        inline static std::unordered_map<std::string, std::vector<ITracker*>>* s_pTrackersByKey = nullptr;
        inline static IAllocator* s_pAllocator = nullptr;
    };

}

