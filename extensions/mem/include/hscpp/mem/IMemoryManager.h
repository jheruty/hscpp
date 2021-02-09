#pragma once

namespace hscpp { namespace mem {

    // Use an IMemoryManager to avoid circular dependency between Ref and MemoryManager.
    class IMemoryManager
    {
    public:
        // An invalid ID, which should be treated like a nullptr.
        constexpr static size_t INVALID_ID = (std::numeric_limits<size_t>::max)();

        // The MemoryManager can return a reference to itself, which has this ID.
        constexpr static size_t MEMORY_MANAGER_ID = (std::numeric_limits<size_t>::max)() - 1;

        virtual ~IMemoryManager() = default;
        virtual uint8_t* GetMemory(uint64_t id) = 0;
        virtual void FreeBlock(uint64_t id, bool bReleaseReservation) = 0;
    };

}}