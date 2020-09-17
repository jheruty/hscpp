#include "hscpp/Feature.h"

namespace hscpp
{

    size_t FeatureHasher::operator()(Feature feature) const
    {
        return static_cast<size_t>(feature);
    }

}