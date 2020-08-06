#pragma once

#include <stdint.h>

#include "hscpp-example-utils/IMemoryManager.h"

// Maps an id to memory.
template <typename T>
struct Ref
{
    size_t id = 0;
    IMemoryManager* pMemoryManager = nullptr;

    T* operator->()
    {
        return reinterpret_cast<T*>(pMemoryManager->GetMemory(id));
    }

    T* operator*()
    {
        return reinterpret_cast<T*>(pMemoryManager->GetMemory(id));
    }

    template <typename U>
    typename std::enable_if<std::is_base_of<U, T>::value, Ref<U>>::type
    As()
    {
        Ref<U> base;
        base.id = id;

        return base;
    }
};