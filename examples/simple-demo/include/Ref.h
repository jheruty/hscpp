#pragma once

#include <stdint.h>
#include "Memory.h"

class BaseRef
{
public:
    uint64_t id;
};

template <typename T>
class Ref : public BaseRef
{
public:
    T* operator->()
    {
        return (T*)::Get(id);
    }
};