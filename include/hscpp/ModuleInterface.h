#pragma once

#include <unordered_map>

#include "hscpp/ModuleSharedState.h"
#include "hscpp/Constructors.h"
#include "hscpp/ITracker.h"

#define HSCPP_API __declspec(dllexport)

namespace hscpp
{
    // Interface into the module. Functions are marked 'virtual' to force using the new module's
    // methods. Otherwise, symbol resolution will be ambiguous, and the main executable will
    // continue to use its own implementation.
    class ModuleInterface
    {
    public:
        virtual void SetTrackersByKey(
            std::unordered_map<std::string, std::vector<ITracker*>>* pTrackersByKey)
        {
            ModuleSharedState::s_pTrackersByKey = pTrackersByKey;
        }

        virtual void SetAllocator(IAllocator* pAllocator)
        {
            ModuleSharedState::s_pAllocator = pAllocator;
        }

        virtual void PerformRuntimeSwap()
        {
            // Get constructors registered within this module.
            auto& constructorsByKey = Constructors::GetConstructorsByKey();

            for (const auto& constructorPair : constructorsByKey)
            {
                // Find tracked objects corresponding to this constructor. If not found, this must
                // be a new class, so no instances have been created yet.
                std::string key = constructorPair.first;

                auto trackedObjectsPair = ModuleSharedState::s_pTrackersByKey->find(key);
                if (trackedObjectsPair != ModuleSharedState::s_pTrackersByKey->end())
                {
                    std::vector<ITracker*>& trackedObjects = trackedObjectsPair->second;
                    std::vector<ITracker*> oldTrackedObjects = trackedObjects;

                    std::vector<std::unique_ptr<SwapInfo>> swapInfos;
                    std::vector<uint64_t> memoryIds;

                    // Free the old objects; they will be swapped out with new instances.
                    size_t nInstances = trackedObjects.size();
                    for (size_t i = 0; i < nInstances; ++i)
                    {
                        ITracker* pTracker = oldTrackedObjects.at(i);

                        swapInfos.push_back(std::make_unique<SwapInfo>());
                        swapInfos.back()->m_Id = i;
                        swapInfos.back()->m_Type = SwapType::BeforeSwap;

                        pTracker->CallSwapHandler(*swapInfos.back());
                        memoryIds.push_back(pTracker->FreeTrackedObject());
                    }

                    trackedObjects.clear();

                    // Create new instances from the new constructors. These will automatically
                    // register themselves into the m_pTrackersByKey map.
                    IConstructor* pConstructor = Constructors::GetConstructor(key);

                    for (size_t i = 0; i < nInstances; ++i)
                    {
                        pConstructor->Construct(memoryIds.at(i));
                        ITracker* pTracker = trackedObjects.at(i);

                        swapInfos.at(i)->m_Type = SwapType::AfterSwap;
                        pTracker->CallSwapHandler(*swapInfos.at(i));
                    }
                }
            }
        }
    };
}

extern "C"
{
    HSCPP_API inline hscpp::ModuleInterface* Hscpp_GetModuleInterface()
    {
        static hscpp::ModuleInterface moduleInterface;
        return &moduleInterface;
    }
}