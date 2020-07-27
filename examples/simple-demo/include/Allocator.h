#pragma once

#include "hscpp/IAllocator.h"
#include "Ref.h"

class TAllocator
{
public:
    template <typename T>
    static Ref<T> Allocate()
    {
        static uint64_t count = 0;
        uint64_t size = sizeof(std::aligned_storage<sizeof(T)>::type);
        uint8_t* mem = ::Allocate(size, ++count);

        T* pT = new (mem) T;

        Ref<T> ref;
        ref.id = count;

        return ref;
    }

    template <typename T>
    static void Free(Ref<T> t)
    {
        t->~T();
        return ::FreeId(t.id);
    }
};

class Allocator : public hscpp::IAllocator
{

public:
    uint8_t* Allocate(uint64_t size, uint64_t id) override;
    uint64_t Free(uint8_t* pMemory) override;
};