#pragma once

namespace hscpp
{
    enum class Feature
    {
        // Enable parsing of hscpp_require_source, hscpp_require_include, hscpp_require_lib,
        // and hscpp_preprocessor_definitions.
        HscppMacros,

        // TODO: implement
        DependentCompilation,
    };
}