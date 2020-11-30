#pragma once

#include <functional>
#include <algorithm>

#include "hscpp/module/CompileTimeString.h"
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

    template <typename T, typename CompileTimeKey>
    class Register
    {
    public:
        Register()
        {
            // This will be executed on module load.
            const char* pKey = CompileTimeKey().ToString();
            hscpp::Constructors::RegisterConstructor<T>(pKey);
        }

        // Unused static may be optimized out. Explicitly call this function to ensure that Register
        // gets initialized.
        void ForceInitialization()
        {}
    };

    //============================================================================
    // Tracker 
    //============================================================================

    template <typename T, typename CompileTimeKey>
    class Tracker : public ITracker
    {
    public:
        // The SwapHandler is an optional, user-set callback that will be called on runtime swaps.
        std::function<void(SwapInfo& swapInfo)> SwapHandler;

        Tracker(const Tracker& rhs) = delete;
        Tracker& operator=(const Tracker& rhs) = delete;

        Tracker(T* pTrackedObj)
        {
            s_Register.ForceInitialization();

            // Pointer to the instance we are tracking.
            m_pTrackedObj = pTrackedObj;

            // Register self.
            const char* pKey = CompileTimeKey().ToString();
            (*ModuleSharedState::s_pTrackersByKey)[pKey].push_back(this);
        }

        ~Tracker()
        {
            // Unregister self.
            const char* pKey = CompileTimeKey().ToString();
            std::vector<ITracker*>& trackers = (*ModuleSharedState::s_pTrackersByKey)[pKey];

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
                return ModuleSharedState::s_pAllocator->Hscpp_FreeSwap(reinterpret_cast<uint8_t*>(m_pTrackedObj));
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
            return CompileTimeKey().ToString();
        }

    private:
        static Register<T, CompileTimeKey> s_Register;
        T* m_pTrackedObj = nullptr;
    };

    template <typename T, typename CompileTimeKey>
    Register<T, CompileTimeKey> Tracker<T, CompileTimeKey>::s_Register;

}

// Forward declare AllocationResolver.
namespace hscpp
{
    class AllocationResolver;
}

#ifndef HSCPP_DISABLE

#define HSCPP_TRACK(type, key) \
/* Allocation resolver will access hscpp_ClassKey and hscpp_ClassTracker. Mark as a friend class,
 * as user may call HSCPP_TRACK in the private area of the class.*/ \
friend class hscpp::AllocationResolver; \
\
/* Cache key length to avoid repeated calls to constexpr method slowing down compilation. This also
 * validates that the key length is <= 128 bytes. */ \
static constexpr hscpp::compile_time::KeylenCache<hscpp::compile_time::Strlen(key)> hscpp_KeylenCache = {}; \
\
/* Split string into segments that fit into a uint64_t and save it as the key.. */\
static constexpr hscpp::compile_time::String<                                                          \
    hscpp::compile_time::StringSegmentToIntegral(key, 0, decltype(hscpp_KeylenCache)::len), \
    hscpp::compile_time::StringSegmentToIntegral(key, 1, decltype(hscpp_KeylenCache)::len), \
    hscpp::compile_time::StringSegmentToIntegral(key, 2, decltype(hscpp_KeylenCache)::len), \
    hscpp::compile_time::StringSegmentToIntegral(key, 3, decltype(hscpp_KeylenCache)::len), \
    hscpp::compile_time::StringSegmentToIntegral(key, 4, decltype(hscpp_KeylenCache)::len), \
    hscpp::compile_time::StringSegmentToIntegral(key, 5, decltype(hscpp_KeylenCache)::len), \
    hscpp::compile_time::StringSegmentToIntegral(key, 6, decltype(hscpp_KeylenCache)::len), \
    hscpp::compile_time::StringSegmentToIntegral(key, 7, decltype(hscpp_KeylenCache)::len), \
    hscpp::compile_time::StringSegmentToIntegral(key, 8, decltype(hscpp_KeylenCache)::len), \
    hscpp::compile_time::StringSegmentToIntegral(key, 9, decltype(hscpp_KeylenCache)::len), \
    hscpp::compile_time::StringSegmentToIntegral(key, 10, decltype(hscpp_KeylenCache)::len), \
    hscpp::compile_time::StringSegmentToIntegral(key, 11, decltype(hscpp_KeylenCache)::len), \
    hscpp::compile_time::StringSegmentToIntegral(key, 12, decltype(hscpp_KeylenCache)::len), \
    hscpp::compile_time::StringSegmentToIntegral(key, 13, decltype(hscpp_KeylenCache)::len), \
    hscpp::compile_time::StringSegmentToIntegral(key, 14, decltype(hscpp_KeylenCache)::len), \
    hscpp::compile_time::StringSegmentToIntegral(key, 15, decltype(hscpp_KeylenCache)::len)> hscpp_ClassKey = {}; \
\
/* Create Tracker to track instance of this class. */ \
hscpp::Tracker<type, decltype(hscpp_ClassKey)> hscpp_ClassTracker = { this };

#define Hscpp_SetSwapHandler(cb) \
hscpp_ClassTracker.SwapHandler = cb;

#define Hscpp_IsSwapping() (*hscpp::ModuleSharedState::s_pbSwapping)

#define hscpp_virtual virtual

#else

#define HSCPP_TRACK(type, key)
#define Hscpp_SetSwapHandler(cb) (void)cb
#define Hscpp_IsSwapping() false
#define hscpp_virtual

#endif