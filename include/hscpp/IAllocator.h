#pragma once

#include <cstdint>

namespace hscpp
{
    class IAllocator
    {
    public:
        virtual ~IAllocator() {};
        virtual uint8_t* Allocate(uint64_t size, uint64_t id) = 0;
        virtual uint64_t Free(uint8_t* pMemory) = 0;
    };

    template <typename T>
    class Allocator;
}
