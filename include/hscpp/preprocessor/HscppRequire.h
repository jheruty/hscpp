#pragma once

#include <vector>
#include <string>

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

        Type type = Type::Unknown;
        std::vector<std::string> values;
    };

}