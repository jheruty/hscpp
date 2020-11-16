#include <iostream>
#include <assert.h>

#include "runtime-require-demo/PrintVariant.h"

void PrintVariant(const Variant& v)
{
    switch (v.GetType())
    {
    case Variant::Type::S8:
        std::cout << v.As<int8_t>();
        break;
    case Variant::Type::S16:
        std::cout << v.As<int16_t>();
        break;
    case Variant::Type::S32:
        std::cout << v.As<int32_t>();
        break;
    case Variant::Type::S64:
        std::cout << v.As<int64_t>();
        break;
    case Variant::Type::U8:
        std::cout << v.As<uint8_t>();
        break;
    case Variant::Type::U16:
        std::cout << v.As<uint16_t>();
        break;
    case Variant::Type::U32:
        std::cout << v.As<uint32_t>();
        break;
    case Variant::Type::U64:
        std::cout << v.As<uint64_t>();
        break;
    case Variant::Type::F32:
        std::cout << v.As<float>();
        break;
    case Variant::Type::F64:
        std::cout << v.As<double>();
        break;
    default:
        assert(false);
        break;
    }

    std::cout << std::endl;
}
