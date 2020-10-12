#pragma once

#include <stdint.h>

class IMemoryManager
{
public:
    virtual uint8_t* GetMemory(uint64_t id) = 0;
};


