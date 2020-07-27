#pragma once

#include <unordered_map>

#include "Memory.h"

static std::unordered_map<uint64_t, uint8_t*> IdToMem;
static std::unordered_map<uint8_t*, uintptr_t> MemToId;

uint8_t* Allocate(uint64_t size, uint64_t id)
{
    uint8_t* mem = new uint8_t[size];
    MemToId[mem] = id;
    IdToMem[id] = mem;

    return mem;
}

uint64_t Free(uint8_t* pMemory)
{
    auto id = MemToId.find(pMemory);
    auto mem = IdToMem.find(id->second);

    auto uid = id->second;

    MemToId.erase(id);
    IdToMem.erase(mem);

    delete[] pMemory;

    return uid;
}

void FreeId(uint64_t uid)
{
    auto mem = IdToMem.find(uid);
    auto id = MemToId.find(mem->second);

    delete[] mem->second;

    MemToId.erase(id);
    IdToMem.erase(mem);
}

uint8_t* Get(uint64_t id)
{
    return IdToMem.find(id)->second;
}
