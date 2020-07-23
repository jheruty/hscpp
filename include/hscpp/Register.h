#pragma once

#include "Constructors.h"

#define HSCPP_REGISTER_HOTSWAPPABLE(T, key) \
namespace\
{\
    struct Registration\
    {\
        Registration()\
        {\
            hscpp::Constructors::RegisterConstructor<T>(key);\
        }\
    };\
\
Registration registration##T;\
}