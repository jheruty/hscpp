#pragma once

namespace hscpp
{
    enum class Feature
    {
        // Enable parsing of hscpp_require_source, hscpp_require_include, and hscpp_require_lib.
        HscppRequire,

        // TODO: implement
        DependentCompilation,
    };
}