#include <unordered_map>
#include <vector>

#include "hscpp/FeatureManager.h"

namespace hscpp
{

    // Features may implicitly enable other features, for example, Feature::DependentCompilation
    // implicitly enables Feature::Preprocessor.
    const static std::unordered_map<Feature, std::vector<Feature>, FeatureHasher> LINKED_FEATURES = {
        { Feature::Preprocessor, { Feature::DependentCompilation } },
    };

    void FeatureManager::EnableFeature(Feature feature)
    {
        m_Features.insert(feature);
    }

    void FeatureManager::DisableFeature(Feature feature)
    {
        auto featureIt = m_Features.find(feature);
        if (featureIt != m_Features.end())
        {
            m_Features.erase(featureIt);
        }
    }

    bool FeatureManager::IsFeatureEnabled(Feature feature)
    {
        auto featureIt = m_Features.find(feature);
        if (featureIt != m_Features.end())
        {
            return true;
        }

        auto linkedFeaturesIt = LINKED_FEATURES.find(feature);
        if (linkedFeaturesIt != LINKED_FEATURES.end())
        {
            for (const auto& linkedFeature : linkedFeaturesIt->second)
            {
                featureIt = m_Features.find(linkedFeature);
                if (featureIt != m_Features.end())
                {
                    return true;
                }
            }
        }

        return false;
    }

}
