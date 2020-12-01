#pragma once

#include <cstdint>
#include <vector>
#include <type_traits>
#include <cstring>

#include "hscpp/module/IAllocator.h"
#include "hscpp/module/AllocationResolver.h"
#include "hscpp-example-utils/Ref.h"
#include "IMemoryManager.h"

// Allow the MemoryManager to return a Ref to itself, with a special id flag.
const static uint64_t MEMORY_MANAGER_ID = (std::numeric_limits<uint64_t>::max)() - 1;

// Memory allocator that is meant to map ids to memory addresses. Kept simple to better demonstrate
// the concept.
class MemoryManager : public hscpp::IAllocator, public IMemoryManager
{
private:
    struct Block
    {
        bool bFree = false;
        uint64_t capacity = 0;
        uint8_t* pMemory = nullptr;
    };

public:
    static Ref<MemoryManager> Create(hscpp::AllocationResolver* pAllocationResolver)
    {
        MemoryManager* pMemoryManager = new MemoryManager();

        pMemoryManager->m_pHscppAllocationResolver = pAllocationResolver;

        Ref<MemoryManager> ref;
        ref.id = MEMORY_MANAGER_ID;
        ref.pMemoryManager = pMemoryManager;

        return ref;
    }

    template <typename T>
    Ref<T> Allocate()
    {
        if (m_pHscppAllocationResolver == nullptr)
        {
            // Hscpp is disabled, so we can allocate with 'new'.
            size_t size = sizeof(typename std::aligned_storage<sizeof(T)>::type);
            size_t iBlock = TakeFirstFreeBlock(size);

            new (m_Blocks.at(iBlock).pMemory) T;

            Ref<T> ref;
            ref.id = iBlock;
            ref.pMemoryManager = this;

            return ref;
        }
        else
        {
            // If hscpp is enabled, we always want to allocate through the hscpp::Hotswapper, as it
            // will use the latest constructor for a given type.
            hscpp::AllocationInfo info;
            m_pHscppAllocationResolver->Allocate<T>(info);

            Ref<T> ref;
            ref.id = info.id;
            ref.pMemoryManager = this;

            return ref;
        }
    }

    template <typename T>
    void Free(Ref<T> ref)
    {
        if (ref.id < m_Blocks.size())
        {
            Block& block = m_Blocks.at(static_cast<size_t>(ref.id));
            block.bFree = true;

            T* pObject = reinterpret_cast<T*>(block.pMemory);
            pObject->~T();

            // Zero out memory to cause crashes when dereferencing dangling pointers.
            std::memset(block.pMemory, 0, static_cast<size_t>(block.capacity));
        }
    }

    template <typename T>
    Ref<T> Place(T* pMemory)
    {
        // Add memory to MemoryManager that has been allocated externally.
        Block block;
        block.bFree = false;
        block.pMemory = reinterpret_cast<uint8_t*>(pMemory);

        size_t iBlock = m_Blocks.size();
        m_Blocks.push_back(block);

        Ref<T> ref;
        ref.id = iBlock;
        ref.pMemoryManager = this;

        return ref;
    }

    uint8_t* GetMemory(uint64_t id) override;

    //============================================================================
    // hscpp::IAllocator implementation.
    //============================================================================
    hscpp::AllocationInfo Hscpp_Allocate(uint64_t size) override;
    hscpp::AllocationInfo Hscpp_AllocateSwap(uint64_t previousId, uint64_t size) override;
    uint64_t Hscpp_FreeSwap(uint8_t* pMemory) override;

private:
    size_t TakeFirstFreeBlock(uint64_t size);

    std::vector<Block> m_Blocks;
    hscpp::AllocationResolver* m_pHscppAllocationResolver = nullptr;
};