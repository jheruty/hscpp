#pragma once

#include <functional>

#include "hscpp/Constructors.h"
#include "hscpp/ModuleSharedState.h"
#include "hscpp/SwapInfo.h"
#include "hscpp/ModuleInterface.h" // Added so it is included in module build.

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

        void ForceInitialization() {};
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

        Tracker(T* pTrackedObj)
        {
            // The compiler may remove statics that are not explicitly referenced.
            s_Register.ForceInitialization();

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

        virtual uint64_t FreeTrackedObject() override
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
                return ModuleSharedState::s_pAllocator->Free(reinterpret_cast<uint8_t*>(m_pTrackedObj));
            }
        }

        virtual void CallSwapHandler(SwapInfo& info) override
        {
            if (SwapHandler != nullptr)
            {
                SwapHandler(info);
            }
        }

        virtual std::string GetKey() override
        {
            return Key;
        }

    private:
        static Register<T, Key> s_Register;
        T* m_pTrackedObj;
    };

    template <typename T, const char* Key>
    hscpp::Register<T, Key> hscpp::Tracker<T, Key>::s_Register;

}

#ifndef HSCPP_DISABLE

#define HSCPP_TRACK(type, key) \
inline static const char hscpp_ClassKey[] = key;\
hscpp::Tracker<type, hscpp_ClassKey> hscpp_ClassTracker = hscpp::Tracker<type, hscpp_ClassKey>(this);

#define HSCPP_SET_SWAP_HANDLER(...) \
hscpp_ClassTracker.SwapHandler = __VA_ARGS__;

#else

#define HSCPP_TRACK(type, key)
#define HSCPP_SET_SWAP_HANDLER(...)

#endif