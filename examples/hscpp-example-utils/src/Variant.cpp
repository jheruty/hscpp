
#include <hscpp/preprocessor/Variant.h>

#include "hscpp-example-utils/Variant.h"

Variant::Variant()
{
    m_Type = Type::S32;
    m_Value.int32_t_ = 0;
}

Variant::Variant(int8_t val)
{
    m_Type = Type::S8;
    m_Value.int8_t_ = val;
}

Variant::Variant(int16_t val)
{
    m_Type = Type::S16;
    m_Value.int16_t_ = val;
}

Variant::Variant(int32_t val)
{
    m_Type = Type::S32;
    m_Value.int32_t_ = val;
}

Variant::Variant(int64_t val)
{
    m_Type = Type::S64;
    m_Value.int64_t_ = val;
}

Variant::Variant(uint8_t val)
{
    m_Type = Type::U8;
    m_Value.uint8_t_ = val;
}

Variant::Variant(uint16_t val)
{
    m_Type = Type::U16;
    m_Value.uint16_t_ = val;
}

Variant::Variant(uint32_t val)
{
    m_Type = Type::U32;
    m_Value.uint32_t_ = val;
}

Variant::Variant(uint64_t val)
{
    m_Type = Type::U64;
    m_Value.uint64_t_ = val;
}

Variant::Variant(float val)
{
    m_Type = Type::F32;
    m_Value.float_ = val;
}

Variant::Variant(double val)
{
    m_Type = Type::F64;
    m_Value.double_ = val;
}

Variant::Type Variant::GetType() const
{
    return m_Type;
}
