#pragma once

#include <stdint.h>

// Interface is needed here to avoid circular dependency between Ref and MemoryManager.
class IMemoryManager
{
public:
    virtual uint8_t* GetMemory(uint64_t id) = 0;
};


