#include <assert.h>

#include "hscpp-example-utils/MemoryManager.h"
#include "hscpp/Hotswapper.h"

uint8_t* MemoryManager::GetMemory(uint64_t id)
{
    if (id < m_Blocks.size())
    {
        return m_Blocks.at(static_cast<size_t>(id)).pMemory;
    }
    else if (id == MEMORY_MANAGER_ID)
    {
        return reinterpret_cast<uint8_t*>(this);
    }

    return nullptr;
}

hscpp::AllocationInfo MemoryManager::Hscpp_Allocate(uint64_t size)
{
    // This will get called when an object instance is created with the hscpp::Hotswapper's
    // GetAllocationResolver()->Allocate<T>() function.
    size_t iBlock = TakeFirstFreeBlock(size);

    hscpp::AllocationInfo info;
    info.id = static_cast<uint64_t>(iBlock);
    info.pMemory = m_Blocks.at(iBlock).pMemory;

    return info;
}

hscpp::AllocationInfo MemoryManager::Hscpp_AllocateSwap(uint64_t previousId, uint64_t size)
{
    // This will be called when an object instance is being swapped by hscpp. That is, an old object
    // instance is being deleted, to be replaced by the object being allocated here.
    if (previousId < m_Blocks.size())
    {
        // We are given the previous memory block that the object to be swapped inhabited. Use
        // the same memory block, such that Ref<T>'s to the object continue working.
        Block& block = m_Blocks.at(static_cast<size_t>(previousId));

        block.bFree = false;
        block.capacity = size;
        block.pMemory = new uint8_t[static_cast<size_t>(size)];

        hscpp::AllocationInfo info;
        info.id = previousId;
        info.pMemory = block.pMemory;

        return info;
    }

    assert(false);
    return hscpp::AllocationInfo();
}

uint64_t MemoryManager::Hscpp_FreeSwap(uint8_t* pMemory)
{
    // This will be called when an object instance is being swapped by hscpp. That is, pMemory will
    // contain a pointer to old object's memory to delete.
    for (size_t i = 0; i < m_Blocks.size(); ++i)
    {
        Block& block = m_Blocks.at(i);
        if (pMemory == block.pMemory)
        {
            block.bFree = true;
            block.capacity = 0;
            delete[] block.pMemory; // Delete the memory to cause obvious faults on bad usage.

            // Return the id of the previous object. This will be passed into Hscpp_AllocateSwap, so
            // that the replacement object can choose to use the same memory location, and allow old
            // Ref<T>'s to continue working.
            return i;
        }
    }

    assert(false);
    return 0;
}

size_t MemoryManager::TakeFirstFreeBlock(uint64_t size)
{
    size_t iBlock = (std::numeric_limits<size_t>::max)();
    for (size_t i = 0; i < m_Blocks.size(); ++i)
    {
        Block& block = m_Blocks.at(i);

        if (block.bFree && size < block.capacity)
        {
            iBlock = i;
            break;
        }
    }

    if (iBlock == (std::numeric_limits<size_t>::max)())
    {
        // No free blocks, allocate a new one.
        iBlock = m_Blocks.size();

        Block block;
        block.bFree = false;
        block.capacity = size;
        block.pMemory = new uint8_t[static_cast<size_t>(size)];

        m_Blocks.push_back(block);
    }

    return iBlock;
}
