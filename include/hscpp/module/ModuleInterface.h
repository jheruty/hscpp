#pragma once

#include <unordered_map>
#include <vector>
#include <assert.h>

#include "hscpp/module/ModuleSharedState.h"
#include "hscpp/module/GlobalUserData.h"
#include "hscpp/module/Constructors.h"
#include "hscpp/module/ITracker.h"

#ifdef _WIN32

#define HSCPP_API __declspec(dllexport)

#else

#define HSCPP_API __attribute__((visibility("default")))

#endif

namespace hscpp
{
    // Interface into the module. Functions are marked 'virtual' to force using the new module's
    // methods. Otherwise, symbol resolution will be ambiguous, and the main executable will
    // continue to use its own implementation.
    class ModuleInterface
    {
    public:
        virtual void SetIsSwapping(bool* pbSwapping)
        {
            ModuleSharedState::s_pbSwapping = pbSwapping;
        }

        virtual void SetTrackersByKey(
            std::unordered_map<std::string, std::vector<ITracker*>>* pTrackersByKey)
        {
            ModuleSharedState::s_pTrackersByKey = pTrackersByKey;
        }

        virtual void SetConstructorsByKey(std::unordered_map<std::string, IConstructor*>* pConstructorsByKey)
        {
            ModuleSharedState::s_pConstructorsByKey = pConstructorsByKey;
        }

        virtual void SetAllocator(IAllocator* pAllocator)
        {
            ModuleSharedState::s_pAllocator = pAllocator;
        }

        virtual void SetGlobalUserData(void* pGlobalUserData)
        {
            GlobalUserData::s_pData = pGlobalUserData;
        }

        virtual std::unordered_map<std::string, IConstructor*> GetModuleConstructorsByKey()
        {
            std::unordered_map<std::string, IConstructor*> constructorsByKey;

            size_t nConstructorKeys = Constructors::GetNumberOfKeys();
            for (size_t iKey = 0; iKey < nConstructorKeys; ++iKey)
            {
                std::string key = Constructors::GetKey(iKey);
                constructorsByKey[key] = Constructors::GetConstructor(key);
            }

            return constructorsByKey;
        }

        virtual void PerformRuntimeSwap()
        {
            *ModuleSharedState::s_pbSwapping = true;

            // Get constructors registered within this module.
            size_t nConstructorKeys = Constructors::GetNumberOfKeys();
            for (size_t iKey = 0; iKey < nConstructorKeys; ++iKey)
            {
                std::string key = Constructors::GetKey(iKey);

                // Patch our global constructors to include the new constructors from this module.
                (*ModuleSharedState::s_pConstructorsByKey)[key] = Constructors::GetConstructor(key);

                // Find tracked objects corresponding to this constructor. If not found, this must
                // be a new class, so no instances have been created yet.
                auto trackersIt = ModuleSharedState::s_pTrackersByKey->find(key);
                if (trackersIt != ModuleSharedState::s_pTrackersByKey->end())
                {
                    // Get tracked objects, and make a copy. As objects are freed, their tracker
                    // will be erased from the trackedObjects vector.
                    std::vector<ITracker*>& trackedObjects = trackersIt->second;
                    std::vector<ITracker*> oldTrackedObjects = trackedObjects;

                    size_t nInstances = trackedObjects.size();

                    std::vector<SwapInfo> swapInfos(nInstances);
                    std::vector<uint64_t> memoryIds(nInstances);

                    // Free the old objects; they will be swapped out with new instances.
                    for (size_t i = 0; i < nInstances; ++i)
                    {
                        swapInfos.at(i).m_Id = i;
                        swapInfos.at(i).m_Phase = SwapPhase::BeforeSwap;

                        ITracker* pTracker = oldTrackedObjects.at(i);

                        pTracker->CallSwapHandler(swapInfos.at(i));
                        memoryIds.at(i) = pTracker->FreeTrackedObject();
                    }

                    // Freeing the tracked objects should have also deleted their tracker, so this
                    // list should now be empty.
                    assert(trackedObjects.empty());

                    // Create new instances from the new constructors. These will have automatically
                    // registered themselves into the m_pTrackersByKey map.
                    IConstructor* pConstructor = Constructors::GetConstructor(key);

                    for (size_t i = 0; i < nInstances; ++i)
                    {
                        pConstructor->AllocateSwap(memoryIds.at(i));

                        // After construction, a new tracker should have been added to trackedObjects.
                        ITracker* pTracker = trackedObjects.at(i);

                        swapInfos.at(i).m_Phase = SwapPhase::AfterSwap;
                        pTracker->CallSwapHandler(swapInfos.at(i));
                        swapInfos.at(i).TriggerInitCb();
                    }
                }
            }

            *ModuleSharedState::s_pbSwapping = false;
        }

        virtual std::vector<Constructors::DuplicateKey> GetDuplicateKeys()
        {
            return Constructors::GetDuplicateKeys();
        }
    };
}

extern "C"
{
    HSCPP_API hscpp::ModuleInterface* Hscpp_GetModuleInterface();
}