#include "hscpp/module/ModuleInterface.h"
#include "hscpp/module/GlobalUserData.h"
#include "hscpp/module/ModuleSharedState.h"


// To avoid needing to force-include many files when doing a runtime compile, the majority of the
// hscpp module is a header-only library. However, there are some situations where this would
// either require hacky workarounds (statics) or may be impossible (Hscpp_GetModuleInterface).
//
// This file is therefore force-included on recompilation to handle those cases.

namespace hscpp
{

    void* GlobalUserData::s_pData = nullptr;

    bool* ModuleSharedState::s_pbSwapping = nullptr;
    std::unordered_map<std::string, std::vector<ITracker*>>* ModuleSharedState::s_pTrackersByKey = nullptr;
    std::unordered_map<std::string, IConstructor*>* ModuleSharedState::s_pConstructorsByKey = nullptr;
    IAllocator* ModuleSharedState::s_pAllocator = nullptr;

}

extern "C"
{

    hscpp::ModuleInterface* Hscpp_GetModuleInterface()
    {
        static hscpp::ModuleInterface moduleInterface;
        return &moduleInterface;
    }

}
