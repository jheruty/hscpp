#pragma once

#include <stdint.h>

#include "hscpp-example-utils/GetMemory.h"

template <typename T>
struct Ref
{
    size_t id = 0;

    T* operator->()
    {
        return reinterpret_cast<T*>(GetMemory(id));
    }

    T* operator*()
    {
        return reinterpret_cast<T*>(GetMemory(id));
    }
};