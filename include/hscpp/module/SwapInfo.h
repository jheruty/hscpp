#pragma once

#include <stdint.h>
#include <assert.h>

#include "hscpp/module/Serializer.h"

namespace hscpp
{
    enum class SwapPhase
    {
        BeforeSwap,
        AfterSwap,
    };

    class SwapInfo
    {
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
        void Save(const std::string& name, T& val)
        {
            switch (m_Phase)
            {
            case SwapPhase::BeforeSwap:
                Serialize(name, val);
                break;
            case SwapPhase::AfterSwap:
                Unserialize(name, val);
                break;
            default:
                assert(false);
                break;
            }
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
        // m_Id and m_Phase are set in ModuleInterface during swapping.
        friend class ModuleInterface;

        SwapPhase m_Phase;
        uint64_t m_Id;
        Serializer m_Serializer;
    };

}


