#pragma once

#include <cstdint>
#include <vector>
#include <type_traits>

#include "hscpp/IAllocator.h"
#include "hscpp/Hotswapper.h"
#include "hscpp-example-utils/Ref.h"
#include "IMemoryManager.h"

const static uint64_t MEMORY_MANAGER_ID = (std::numeric_limits<uint64_t>::max)() - 1;

// Memory allocator that is meant to map ids to memory addresses. Kept simple to better demonstrate
// the concept.
class MemoryManager : public hscpp::IAllocator, public IMemoryManager
{
private:
    struct Block
    {
        bool bFree = false;
        bool bExternallyOwned = false;
        size_t capacity = 0;
        uint8_t* pMemory = nullptr;
    };

public:
    static Ref<MemoryManager> Create(hscpp::Hotswapper* pSwapper)
    {
        MemoryManager* pMemoryManager = new MemoryManager();

        pMemoryManager->m_pSwapper = pSwapper;

        Ref<MemoryManager> ref;
        ref.id = MEMORY_MANAGER_ID;
        ref.pMemoryManager = pMemoryManager;

        return ref;
    }

    template <typename T>
    Ref<T> Allocate()
    {
        if (m_pSwapper == nullptr)
        {
            size_t size = sizeof(std::aligned_storage<sizeof(T)>::type);
            size_t iBlock = TakeFirstFreeBlock(size);

            T* pT = new (m_Blocks.at(iBlock).pMemory) T;

            Ref<T> ref;
            ref.id = iBlock;
            ref.pMemoryManager = this;

            return ref;
        }
        else
        {
            // If hscpp is enabled, we always want to allocate through the hscpp::Hotswapper, as it
            // will use the latest constructor for a given type.
            hscpp::AllocationInfo info = m_pSwapper->Allocate<T>();

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
            Block& block = m_Blocks.at(ref.id);
            block.bFree = true;

            T* pObject = reinterpret_cast<T*>(block.pMemory);
            pObject->~T();

            // Zero out memory to cause crashes when dereferencing dangling pointers.
            std::memset(block.pMemory, 0, block.capacity);
        }
    }

    template <typename T>
    Ref<T> Place(T* pMemory)
    {
        Block block;
        block.bFree = false;
        block.bExternallyOwned = true;
        block.pMemory = reinterpret_cast<uint8_t*>(pMemory);

        size_t iBlock = m_Blocks.size();
        m_Blocks.push_back(block);

        Ref<T> ref;
        ref.id = iBlock;
        ref.pMemoryManager = this;

        return ref;
    }

    uint8_t* GetMemory(size_t id) override;

    //============================================================================
    // hscpp::IAllocator implementation.
    //============================================================================
    hscpp::AllocationInfo Hscpp_Allocate(uint64_t size) override;
    hscpp::AllocationInfo Hscpp_AllocateSwap(uint64_t previousId, uint64_t size) override;
    uint64_t Hscpp_Free(uint8_t* pMemory) override;

private:
    size_t TakeFirstFreeBlock(size_t size);

    std::vector<Block> m_Blocks;
    hscpp::Hotswapper* m_pSwapper = nullptr;
};