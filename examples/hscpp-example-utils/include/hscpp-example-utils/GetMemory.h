#pragma once

#include <stdint.h>

// Put this in it's own file to get around Ref.h and MemoryManager.h circular dependency.
uint8_t* GetMemory(size_t id);