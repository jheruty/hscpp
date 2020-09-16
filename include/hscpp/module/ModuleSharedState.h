#pragma once

#include <unordered_map>
#include <string>

#include "hscpp/module/IAllocator.h"

namespace hscpp
{
    class ITracker;
    class IConstructor;

    // Templated to allow static variables in header.
    template <int T = 0>
    class ModuleSharedStateImpl
    {
    public:
        // Internal global state required by hscpp. Modify at your own peril.
        static bool* s_pbSwapping;
        static std::unordered_map<std::string, std::vector<ITracker*>>* s_pTrackersByKey;
        static std::unordered_map<std::string, IConstructor*>* s_pConstructorsByKey;
        static IAllocator* s_pAllocator;
    };

    template <int T>
    bool* ModuleSharedStateImpl<T>::s_pbSwapping = nullptr;

    template <int T>
    std::unordered_map<std::string, std::vector<ITracker*>>* ModuleSharedStateImpl<T>::s_pTrackersByKey = nullptr;

    template <int T>
    std::unordered_map<std::string, IConstructor*>* ModuleSharedStateImpl<T>::s_pConstructorsByKey = nullptr;

    template <int T>
    IAllocator* ModuleSharedStateImpl<T>::s_pAllocator = nullptr;

    // Typedef for easier usage; in the future, C++17 inline statics makes the templating of this
    // class unnecessary.
    typedef ModuleSharedStateImpl<> ModuleSharedState;

}

