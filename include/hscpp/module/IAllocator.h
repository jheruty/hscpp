#pragma once

#include <cstdint>
#include <limits>

namespace hscpp
{
    struct AllocationInfo
    {
        uint64_t id = (std::numeric_limits<uint64_t>::max)();
        uint8_t* pMemory = nullptr;
    };

    class IAllocator
    {
    public:
        virtual ~IAllocator() = default;

        // Called when an object is first constructed.
        virtual AllocationInfo Hscpp_Allocate(uint64_t size) = 0;

        // Called when an object is undergoing a runtime swap, and a new object is being constructed
        // to replace an old implementation.
        virtual AllocationInfo Hscpp_AllocateSwap(uint64_t previousId, uint64_t size) = 0;

        // Called when an object is freed during a runtime swap, and should return the old object's id.
        virtual uint64_t Hscpp_FreeSwap(uint8_t* pMemory) = 0;
    };

    template <typename T>
    class Allocator;
}
