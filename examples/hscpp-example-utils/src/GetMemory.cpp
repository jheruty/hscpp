#include "hscpp-example-utils/GetMemory.h"
#include "hscpp-example-utils/MemoryManager.h"

uint8_t* GetMemory(size_t id)
{
    return MemoryManager::Instance().GetMemory(id);
}
