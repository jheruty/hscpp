#pragma once

#include <vector>
#include <string>

#include "hscpp/preprocessor/LangError.h"

namespace hscpp
{

    struct HscppRequire
    {
        enum class Type
        {
            Unknown,
            Source,
            IncludeDir,
            Library,
            LibraryDir,
            PreprocessorDef,
        };

        std::string name; // ex. hscpp_require_source
        size_t line = LangError::NO_VALUE;

        Type type = Type::Unknown;
        std::vector<std::string> values;
    };

}