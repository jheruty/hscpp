#pragma once

#include <unordered_set>

#include "hscpp/Feature.h"

namespace hscpp
{

    class FeatureManager
    {
    public:
        void EnableFeature(Feature feature);
        void DisableFeature(Feature feature);
        bool IsFeatureEnabled(Feature feature);

    private:
        std::unordered_set<Feature, FeatureHasher> m_Features;
    };

}