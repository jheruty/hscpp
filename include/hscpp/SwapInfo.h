#pragma once

#include <stdint.h>

#include "hscpp/Serializer.h"

namespace hscpp
{
    enum class SwapPhase
    {
        BeforeSwap,
        AfterSwap,
    };

    class SwapInfo
    {
        friend class ModuleInterface;

    public:
        SwapPhase Phase() const
        {
            return m_Phase;
        }

        uint64_t Id() const
        {
            return m_Id;
        }

        template <typename T>
        void Serialize(const std::string& name, const T& val)
        {
            m_Serializer.Serialize(name, val);
        }

        template <typename T>
        bool Unserialize(const std::string& name, T& val)
        {
            return m_Serializer.Unserialize(name, val);
        }

    private:
        SwapPhase m_Phase;
        uint64_t m_Id;
        Serializer m_Serializer;
    };

}


