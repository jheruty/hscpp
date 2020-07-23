#pragma once

#include <unordered_map>
#include <typeindex>

namespace hscpp
{
    class SharedModuleMemory
    {
    public:
        // TODO keep track of number of destructors called per type.

    private:
        std::unordered_map<std::type_index, int> m_nConstructorsPending;
    };
}