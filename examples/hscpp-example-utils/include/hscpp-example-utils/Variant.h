#pragma once

#include <assert.h>
#include <stdint.h>

// Simple class to be used by examples.
class Variant
{
public:
    enum class Type
    {
        S8,
        S16,
        S32,
        S64,
        U8,
        U16,
        U32,
        U64,
        F32,
        F64,
    };

    Variant();
    explicit Variant(int8_t val);
    explicit Variant(int16_t val);
    explicit Variant(int32_t val);
    explicit Variant(int64_t val);
    explicit Variant(uint8_t val);
    explicit Variant(uint16_t val);
    explicit Variant(uint32_t val);
    explicit Variant(uint64_t val);
    explicit Variant(float val);
    explicit Variant(double val);

    Type GetType() const;

    template <typename T>
    T As() const
    {
        switch (m_Type)
        {
        case Type::S8:
            return static_cast<T>(m_Value.int8_t_);
        case Type::S16:
            return static_cast<T>(m_Value.int16_t_);
        case Type::S32:
            return static_cast<T>(m_Value.int32_t_);
        case Type::S64:
            return static_cast<T>(m_Value.int64_t_);
        case Type::U8:
            return static_cast<T>(m_Value.uint8_t_);
        case Type::U16:
            return static_cast<T>(m_Value.uint16_t_);
        case Type::U32:
            return static_cast<T>(m_Value.uint32_t_);
        case Type::U64:
            return static_cast<T>(m_Value.uint64_t_);
        case Type::F32:
            return static_cast<T>(m_Value.float_);
        case Type::F64:
            return static_cast<T>(m_Value.double_);
        default:
            assert(false);
            return T();
        }
    }

private:
    union Value
    {
        int8_t int8_t_;
        int16_t int16_t_;
        int32_t int32_t_;
        int64_t int64_t_;
        uint8_t uint8_t_;
        uint16_t uint16_t_;
        uint32_t uint32_t_;
        uint64_t uint64_t_;
        float float_;
        double double_;
    };

    Type m_Type;
    Value m_Value;
};
