#include "hscpp/module/GlobalUserData.h"
#include "hscpp/module/ModuleSharedState.h"

namespace hscpp
{
    // ModuleSharedState
    bool* ModuleSharedState::s_pbSwapping = nullptr;
    std::unordered_map<std::string, std::vector<ITracker*>>* ModuleSharedState::s_pTrackersByKey = nullptr;
    std::unordered_map<std::string, IConstructor*>* ModuleSharedState::s_pConstructorsByKey = nullptr;
    IAllocator* ModuleSharedState::s_pAllocator = nullptr;

    // GlobalUserData
    void* GlobalUserData::s_pData = nullptr;
}