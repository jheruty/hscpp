#include "catch/catch.hpp"

#include "hscpp/FeatureManager.h"

namespace hscpp { namespace test
{

    TEST_CASE("FeatureManager is functional.")
    {
        hscpp::FeatureManager featureManager;

        SECTION("Enabling and disabling features works.")
        {
            REQUIRE_FALSE(featureManager.IsFeatureEnabled(Feature::ManualCompilationOnly));

            featureManager.EnableFeature(Feature::ManualCompilationOnly);
            REQUIRE(featureManager.IsFeatureEnabled(Feature::ManualCompilationOnly));

            featureManager.DisableFeature(Feature::ManualCompilationOnly);
            REQUIRE_FALSE(featureManager.IsFeatureEnabled(Feature::ManualCompilationOnly));
        }

        SECTION("Preprocessor is implicitly enabled when DependentCompilation is enabled.")
        {
            REQUIRE_FALSE(featureManager.IsFeatureEnabled(Feature::Preprocessor));
            REQUIRE_FALSE(featureManager.IsFeatureEnabled(Feature::DependentCompilation));

            featureManager.EnableFeature(Feature::DependentCompilation);
            REQUIRE(featureManager.IsFeatureEnabled(Feature::Preprocessor));
            REQUIRE(featureManager.IsFeatureEnabled(Feature::DependentCompilation));

            featureManager.DisableFeature(Feature::DependentCompilation);
            REQUIRE_FALSE(featureManager.IsFeatureEnabled(Feature::Preprocessor));
            REQUIRE_FALSE(featureManager.IsFeatureEnabled(Feature::DependentCompilation));

            featureManager.EnableFeature(Feature::DependentCompilation);
            featureManager.EnableFeature(Feature::Preprocessor);
            featureManager.DisableFeature(Feature::DependentCompilation);
            REQUIRE(featureManager.IsFeatureEnabled(Feature::Preprocessor));
            REQUIRE_FALSE(featureManager.IsFeatureEnabled(Feature::DependentCompilation));
        }
    }

}}