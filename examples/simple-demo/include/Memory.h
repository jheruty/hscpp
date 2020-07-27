#pragma once

#include <type_traits>

#include "hscpp/IAllocator.h"

uint8_t* Allocate(uint64_t size, uint64_t id);
uint64_t Free(uint8_t* pMemory);
void FreeId(uint64_t id);
uint8_t* Get(uint64_t id);
