#pragma once

#include <functional>

#include "hscpp/module/Constructors.h"
#include "hscpp/module/ModuleSharedState.h"
#include "hscpp/module/SwapInfo.h"
#include "hscpp/module/ModuleInterface.h" // Added so it is included in module build.
#include "hscpp/module/PreprocessorMacros.h" // Added so macros are available when using a tracked class.

namespace hscpp
{

    //============================================================================
    // Register
    //============================================================================

    template <typename T, const char* Key>
    class Register
    {
    public:
        Register()
        {
            // This will be executed on module load.
            hscpp::Constructors::RegisterConstructor<T>(Key);
        }
    };

    //============================================================================
    // Tracker 
    //============================================================================

    template <typename T, const char* Key>
    class Tracker : public ITracker
    {
    public:
        // The SwapHandler is an optional, user-set callback that will be called on runtime swaps.
        std::function<void(SwapInfo& swapInfo)> SwapHandler;

        Tracker(const Tracker& rhs) = delete;
        Tracker& operator=(const Tracker& rhs) = delete;

        explicit Tracker(T* pTrackedObj)
        {
            // Pointer to the instance we are tracking.
            m_pTrackedObj = pTrackedObj;

            // Register self.
            (*ModuleSharedState::s_pTrackersByKey)[Key].push_back(this);
        }

        ~Tracker()
        {
            // Unregister self.
            std::vector<ITracker*>& trackers = (*ModuleSharedState::s_pTrackersByKey)[Key];

            auto trackerIt = std::find(trackers.begin(), trackers.end(), this);
            if (trackerIt != trackers.end())
            {
                trackers.erase(trackerIt);
            }
        }

        uint64_t FreeTrackedObject() override
        {
            // Destroying the tracked object will also destroy the tracker it owns.
            if (ModuleSharedState::s_pAllocator == nullptr)
            {
                delete m_pTrackedObj;
                return 0;
            }
            else
            {
                m_pTrackedObj->~T();
                return ModuleSharedState::s_pAllocator->Hscpp_Free(reinterpret_cast<uint8_t*>(m_pTrackedObj));
            }
        }

        void CallSwapHandler(SwapInfo& info) override
        {
            if (SwapHandler != nullptr)
            {
                SwapHandler(info);
            }
        }

        std::string GetKey() override
        {
            return Key;
        }

    private:
        inline static Register<T, Key> s_Register;
        T* m_pTrackedObj = nullptr;
    };

}

// Forward declare AllocationResolver.
namespace hscpp
{
    class AllocationResolver;
}

#ifndef HSCPP_DISABLE

#define HSCPP_TRACK(type, key) \
friend class hscpp::AllocationResolver; \
inline static const char hscpp_ClassKey[] = key;\
hscpp::Tracker<type, hscpp_ClassKey> hscpp_ClassTracker = hscpp::Tracker<type, hscpp_ClassKey>(this);

#define Hscpp_SetSwapHandler(...) \
hscpp_ClassTracker.SwapHandler = __VA_ARGS__;

#define Hscpp_IsSwapping() (*hscpp::ModuleSharedState::s_pbSwapping)

#define hscpp_virtual virtual

#else

#define HSCPP_TRACK(type, key)
#define Hscpp_SetSwapHandler(...)
#define Hscpp_IsSwapping() false
#define hscpp_virtual

#endif