#pragma once

#include <unordered_map>

#include "hscpp/ModuleSharedState.h"
#include "hscpp/Constructors.h"
#include "hscpp/Tracker.h"

namespace hscpp
{
    class __declspec(dllexport) ModuleInterface
    {
    public:
        static void SetTrackersByKey(
            std::unordered_map<std::string, std::vector<ITracker*>>* pTrackersByKey)
        {
            ModuleSharedState::s_pTrackersByKey = pTrackersByKey;
        }

        static void SetAllocator(IAllocator* pAllocator)
        {
            ModuleSharedState::s_pAllocator = pAllocator;
        }

        static void PerformRuntimeSwap()
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

                    // Free the old objects; they will be swapped out with new instances.
                    size_t nInstances = trackedObjects.size();
                    for (ITracker* pTracker : trackedObjects)
                    {
                        pTracker->FreeTrackedObject();
                    }

                    trackedObjects.clear();

                    // Create new instances from the new constructors. These will automatically
                    // register themselves into the m_pTrackersByKey map.
                    IConstructor* pConstructor = Constructors::GetConstructor(key);

                    for (size_t i = 0; i < nInstances; ++i)
                    {
                        pConstructor->Construct();
                    }
                }
            }
        }
    };

}