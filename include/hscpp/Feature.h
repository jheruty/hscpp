#pragma once

#include <cstddef>

namespace hscpp
{

    enum class Feature
    {
        // Enable parsing of hscpp_require_source, hscpp_require_include, hscpp_require_lib,
        // hscpp_preprocessor_definitions, and hscpp_module.
        Preprocessor,

        // #TODO
        DependentCompilation,

        // Do not automatically trigger compilation on file changes. User must call the
        // hscpp::Hotswapper's TriggerManualBuild method.
        ManualCompilationOnly,
    };

    class FeatureHasher
    {
    public:
        size_t operator()(Feature feature) const;
    };

}