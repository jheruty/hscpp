#include "hscpp/FsPathHasher.h"

namespace hscpp
{

    size_t FsPathHasher::operator()(const fs::path& path) const
    {
        return fs::hash_value(path);
    }

}
