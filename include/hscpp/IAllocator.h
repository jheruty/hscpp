#pragma once

#include <cstdint>

namespace hscpp
{
    class IAllocator
    {
    public:
        virtual uintptr_t Allocate(uint64_t size) = 0;
        virtual void Free(uintptr_t id) = 0;
    };
}
