#pragma once

#include <iostream>
#include <stdint.h>
#include <type_traits>

#include "hscpp/IAllocator.h"
#include "hscpp/ModuleSharedState.h"
#include "hscpp/ModuleManager.h"

namespace hscpp
{

    // Use the member detector idiom https://en.wikibooks.org/wiki/More_C++_Idioms/Member_Detector
    // If T contains an hscpp_ClassTracker, then we assume it has been registered for tracking with
    // HSCPP_TRACK and should be constructed through an hscpp Constructor.
    template<typename T>
    class IsTracked
    {
        struct Fallback
        { 
            int hscpp_ClassTracker; 
        };

        // Derived will definitely have a hscpp_ClassTracker member, either through Fallback, or
        // through both Fallback and T.
        struct Derived : T, Fallback
        {};

        typedef char NoType[1];
        typedef char YesType[2];

        template<typename U, U> struct Check;

        // In the following function, "typename U" refers to the type "pointer to an int member in
        // a Fallback struct", and "U" refers to "address of hscpp_ClassTracker in Derived." This can
        // only happen if &Derived::hscpp_ClassTracker is unambiguous (i.e. T does not have a
        // hscpp_ClassTracker member).
        template<typename U>
        static NoType& Test(Check<int Fallback::*, &U::hscpp_ClassTracker>*);

        template<typename U>
        static YesType& Test(...);

    public:
        enum { no = (sizeof(Test<Derived>(0)) == sizeof(NoType)) };
        enum { yes = (sizeof(Test<Derived>(0)) == sizeof(YesType)) };
    };

    class AllocationResolver
    {
    public:
        AllocationResolver(ModuleManager* pModuleManager);

        template <typename T>
        typename std::enable_if<IsTracked<T>::yes, AllocationInfo>::type
        Allocate()
        {
            // This type has an hscpp_ClassTracker member, and it is assumed it has been registered
            // with HSCPP_TRACK. Allocate it using an hscpp Constructor.
            const char* pKey = T::hscpp_ClassKey;
            return m_pModuleManager->Allocate(pKey);
        }

        template <typename T>
        typename std::enable_if<IsTracked<T>::no, AllocationInfo>::type
        Allocate()
        {
            IAllocator* pAllocator = m_pModuleManager->GetAllocator();

            // This is a non-tracked type. Perform a normal allocation.
            if (pAllocator != nullptr)
            {
                uint64_t size = sizeof(std::aligned_storage<sizeof(T)>::type);
                return pAllocator->Hscpp_Allocate(size);
            }

            AllocationInfo info;
            info.pMemory = reinterpret_cast<uint8_t*>(new T());
            
            return info;
        }

    private:
        ModuleManager* m_pModuleManager = nullptr;
    };

}