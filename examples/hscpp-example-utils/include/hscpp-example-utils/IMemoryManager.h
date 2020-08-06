#pragma once

#include <stdint.h>

class IMemoryManager
{
public:
    virtual uint8_t* GetMemory(size_t id) = 0;
};


