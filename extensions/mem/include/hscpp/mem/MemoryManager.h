#pragma once

#include <vector>
#include <cstdint>
#include <limits>

#include "hscpp/module/IAllocator.h"
#include "hscpp/module/AllocationResolver.h"
#include "hscpp/mem/Ref.h"
#include "hscpp/mem/IMemoryManager.h"

namespace hscpp { namespace mem {

    class MemoryManager : public IMemoryManager, public IAllocator
    {
        // Use the Create() function to make a new MemoryManager.
        MemoryManager(const MemoryManager&) = delete;

    public:
        struct Config
        {
            Config();

            AllocationResolver* pAllocationResolver = nullptr;

            // Number of Blocks to reserve on initialization.
            uint64_t reservedBlocks = 100;

            // Overridable Allocate/Free functions.
            std::function<uint8_t*(uint64_t size)> AllocateCb;
            std::function<void(uint8_t* pMemory)> FreeCb;
        };

        static UniqueRef<MemoryManager> Create(const Config& config = Config());

        template <typename T>
        UniqueRef<T> Allocate();

        // Free is handled by the UniqueRef when it goes out of scope. The UniqueRef will handle
        // calling the destructor.

        uint64_t GetNumBlocks() const;

    private:
        struct BlockHeader
        {
            uint64_t iBlock = INVALID_ID;
        };

        struct Block
        {
            // Get BlockHeader at location: pMemory - sizeof(BlockHeader)
            uint8_t* pMemory = nullptr;
        };

        AllocationResolver* m_pAllocationResolver = nullptr;

        std::function<uint8_t*(uint64_t size)> m_AllocateCb;
        std::function<void(uint8_t* pMemory)> m_FreeCb;

        std::vector<Block> m_Blocks;

        uint64_t m_iFreeBlocksBegin = 0;
        std::vector<uint64_t> m_FreeBlockIndices;
        std::vector<uint64_t> m_FreeBlockIndexByBlock;

        MemoryManager() = default;

        // hscpp::mem::IMemoryManager
        // Used by Ref to get underlying memory for a given id. Note that the returned pointer
        // may change for the same id, should a runtime swap take place.
        uint8_t* GetMemory(uint64_t id) override;
        void FreeBlock(uint64_t iBlock, bool bReleaseReservation) override;

        // hscpp::IAllocator
        AllocationInfo Hscpp_Allocate(uint64_t size) override;
        AllocationInfo Hscpp_AllocateSwap(uint64_t previousId, uint64_t size) override;
        uint64_t Hscpp_FreeSwap(uint8_t* pMemory) override;

        // Helper methods
        uint8_t* AllocateBlock(uint64_t size, uint64_t iBlock);
        uint64_t ReserveFirstFreeBlock();
    };

    template<typename T>
    UniqueRef<T> MemoryManager::Allocate()
    {
        UniqueRef<T> ref;
        uint64_t iBlock = IMemoryManager::INVALID_ID;

        if (m_pAllocationResolver != nullptr)
        {
            // hscpp is active, allocate through the hscpp::AllocationResolver. This will ultimately
            // call back into this class, through Hscpp_Allocate.
            AllocationInfo info;
            m_pAllocationResolver->Allocate<T>(info);
            iBlock = info.id;
        }
        else
        {
            // hscpp is inactive, allocate directly.
            uint64_t size = sizeof(typename std::aligned_storage<sizeof(T)>::type);

            iBlock = ReserveFirstFreeBlock();
            uint8_t* pMemory = AllocateBlock(size, iBlock);
            new (pMemory) T;
        }

        ref.m_Id = iBlock;
        ref.m_pMemoryManager = this;
        return ref;
    }

}}