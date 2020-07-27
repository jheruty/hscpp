#pragma once

#include "Allocator.h"

uint8_t* Allocator::Allocate(uint64_t size, uint64_t id)
{
    return ::Allocate(size, id);
}

uint64_t Allocator::Free(uint8_t* pMemory)
{
    return ::Free(pMemory);
}
