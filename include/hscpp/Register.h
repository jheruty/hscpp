#pragma once

#include <functional>

#include "Constructors.h"

namespace hscpp
{


    template <typename T, const char* Key>
    class Register
    {
    public:
        Register()
        {
            // This will be executed on module load.
            hscpp::Constructors::RegisterConstructor<T>(Key);
        }

        static int s_ForceInitialization;
    };

    template <typename T, const char* Key>
    int hscpp::Register<T, Key>::s_ForceInitialization;

    template <typename T, const char* Key>
    class Tracker
    {
    public:
        std::function<void()> OnBeforeSwap;
        std::function<void()> OnAfterSwap;

        Tracker(T* pTrackedObj)
        {
            // The compiler may remove statics that are not explicitly used.
            Register<T, Key>::s_ForceInitialization = 0xFF;

            // Pointer to the instance we are tracking.
            m_pTrackedObj = pTrackedObj;

            // TODO: Register with module memory.
        }

        ~Tracker()
        {
            // TODO: Deregister with module memory. 
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

#define HSCPP_ON_BEFORE_SWAP(...) \
hscpp_ClassTracker.OnBeforeSwap = __VA_ARGS__;

#define HSCPP_ON_AFTER_SWAP(...) \
hscpp_ClassTracker.OnAfterSwap = __VA_ARGS__;

#else

#define HSCPP_TRACK(type, key)
#define HSCPP_ON_BEFORE_SWAP(...)
#define HSCPP_ON_AFTER_SWAP(...)

#endif