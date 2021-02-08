#include "hscpp/mem/MemoryManager.h"

namespace hscpp { namespace mem {

    UniqueRef<MemoryManager> MemoryManager::Create(const Config& config /*=Config()*/)
    {
        MemoryManager* pMemoryManager = new MemoryManager();
        pMemoryManager->m_pAllocationResolver = config.pAllocationResolver;
        pMemoryManager->m_AllocateCb = config.AllocateCb;
        pMemoryManager->m_FreeCb = config.FreeCb;
        pMemoryManager->m_Blocks.reserve(config.reservedBlocks);
        pMemoryManager->m_FreeBlockIndices.reserve(config.reservedBlocks);
        pMemoryManager->m_FreeBlockIndexByBlock.reserve(config.reservedBlocks);

        UniqueRef<MemoryManager> ref;
        ref.m_pMemoryManager = pMemoryManager;
        ref.m_Id = IMemoryManager::MEMORY_MANAGER_ID;

        return std::move(ref);
    }

    uint64_t MemoryManager::GetNumBlocks() const
    {
        return m_iFreeBlocksBegin;
    }

    uint8_t* MemoryManager::GetMemory(uint64_t id)
    {
        switch (id)
        {
            case IMemoryManager::INVALID_ID:
                return nullptr;
            case IMemoryManager::MEMORY_MANAGER_ID:
                return reinterpret_cast<uint8_t*>(this);
            default:
                return m_Blocks.at(id).pMemory;
        }
    }

    void MemoryManager::FreeBlock(uint64_t iBlock, bool bReleaseReservation)
    {
        switch (iBlock)
        {
            case IMemoryManager::INVALID_ID:
                break; // Equivalent to deleting a nullptr.
            case IMemoryManager::MEMORY_MANAGER_ID:
                break; // MemoryManager instance does not use any Block.
            default:
                m_FreeCb(m_Blocks.at(iBlock).pMemory - sizeof(BlockHeader));
                m_Blocks.at(iBlock).pMemory = nullptr;

                if (bReleaseReservation)
                {
                    // Get the index of the last Block that is not free, in the free Block list.
                    size_t iLastUsedBlock = m_FreeBlockIndices.at(m_iFreeBlocksBegin - 1);

                    // Map the Block index to the free Block list.
                    size_t iFreeBlock = m_FreeBlockIndexByBlock.at(iBlock);

                    // Swap freed Block with last taken Block before the free Blocks. Since the
                    // number of free Blocks will be decremented, this will push the freed Block
                    // into the free list.
                    std::swap(m_FreeBlockIndices.at(iFreeBlock), m_FreeBlockIndices.at(m_iFreeBlocksBegin - 1));

                    // Update the free Block list mapping.
                    std::swap(m_FreeBlockIndexByBlock.at(iBlock), m_FreeBlockIndexByBlock.at(iLastUsedBlock));

                    m_iFreeBlocksBegin--;
                }

                break;
        }
    }

    AllocationInfo MemoryManager::Hscpp_Allocate(uint64_t size)
    {
        // Performing a generic allocation through hscpp.
        uint64_t iBlock = ReserveFirstFreeBlock();
        uint8_t* pMemory = AllocateBlock(size, iBlock);

        AllocationInfo info;
        info.id = iBlock;
        info.pMemory = pMemory;

        return info;
    }

    AllocationInfo MemoryManager::Hscpp_AllocateSwap(uint64_t previousId, uint64_t size)
    {
        // Performing a runtime swap of an HSCPP_TRACK object. Reuse the old Block, so that old
        // Refs will now refer to the newly allocated class.
        uint64_t iBlock = previousId;
        AllocateBlock(size, iBlock);

        AllocationInfo info;
        info.id = iBlock;
        info.pMemory = m_Blocks.at(iBlock).pMemory;

        return info;
    }

    uint64_t MemoryManager::Hscpp_FreeSwap(uint8_t* pMemory)
    {
        // Performing a free during a runtime swap. Return the old object id so that
        // HscppAllocateSwap knows the previous id of the deleted object. The Block's
        // memory will be freed, but its id will still be reserved.
        uint64_t iBlock = reinterpret_cast<BlockHeader*>(pMemory - sizeof(BlockHeader))->iBlock;
        FreeBlock(iBlock, false);

        return iBlock;
    }

    uint8_t* MemoryManager::AllocateBlock(uint64_t size, uint64_t iBlock)
    {
        // Allocate sizeof(BlockHeader) extra space, allowing Block info to be saved alongside
        // the pointer. This makes it possible to quickly find the index during an Hscpp_FreeSwap.
        uint8_t* pMemory = m_AllocateCb(size + sizeof(BlockHeader));

        reinterpret_cast<BlockHeader*>(pMemory)->iBlock = iBlock;

        // Return memory past the BlockHeader.
        m_Blocks.at(iBlock).pMemory = pMemory + sizeof(BlockHeader);
        return m_Blocks.at(iBlock).pMemory;
    }

    uint64_t MemoryManager::ReserveFirstFreeBlock()
    {
        if (m_iFreeBlocksBegin == m_FreeBlockIndices.size())
        {
            // No free blocks remain, push back a new one.
            m_FreeBlockIndices.push_back(m_iFreeBlocksBegin);
            m_FreeBlockIndexByBlock.push_back(m_iFreeBlocksBegin);
            m_Blocks.push_back(Block());
        }

        m_iFreeBlocksBegin++;
        return m_FreeBlockIndices.at(m_iFreeBlocksBegin - 1);
    }

    MemoryManager::Config::Config()
    {
        AllocateCb = [](uint64_t size) {
            return new uint8_t[size];
        };

        FreeCb = [](uint8_t* pMemory) {
            delete[] pMemory;
        };
    }
}}