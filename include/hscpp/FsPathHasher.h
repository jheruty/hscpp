#pragma once

#include "hscpp/Platform.h"

namespace hscpp
{

    class FsPathHasher
    {
    public:
        size_t operator()(const fs::path& path) const;
    };

}