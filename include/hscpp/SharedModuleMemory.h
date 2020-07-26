#pragma once

#include <unordered_map>
#include <vector>

#include "hscpp/Constructors.h"
#include "hscpp/ITracker.h"

namespace hscpp
{

    // All functions implemented inline, so that this is compiled into a hotswapped module simply
    // by including Registration.h
    class SharedModuleMemory
    {
    public:        
        _declspec(dllexport) static void SetTrackersByKey(
            std::unordered_map<std::string, std::vector<ITracker*>>* pTrackersByKey)
        {
            // Trackers are shared across all modules.
            m_pTrackersByKey = pTrackersByKey;
        }

        _declspec(dllexport) static void SwapObjects()
        {
            // Get constructors registered within this module.
            auto constructorsByKey = Constructors::GetConstructorsByKey();

            for (const auto& constructorPair : constructorsByKey)
            {
                // Find tracked objects corresponding to this constructor. If not found, this must
                // be a new class, so no instances have been created yet.
                std::string key = constructorPair.first;
                
                auto trackedObjectsPair = m_pTrackersByKey->find(key);
                if (trackedObjectsPair != m_pTrackersByKey->end())
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

        static void RegisterTracker(ITracker* pTracker)
        {
            (*m_pTrackersByKey)[pTracker->GetKey()].push_back(pTracker);
        }

        static void UnregisterTracker(ITracker* pTracker)
        {
            std::vector<ITracker*>& trackers = (*m_pTrackersByKey)[pTracker->GetKey()];

            auto trackerIt = std::find(trackers.begin(), trackers.end(), pTracker);
            if (trackerIt != trackers.end())
            {
                trackers.erase(trackerIt);
            }
        }

    private:
        inline static std::unordered_map<std::string, std::vector<ITracker*>>* m_pTrackersByKey;
    };

}