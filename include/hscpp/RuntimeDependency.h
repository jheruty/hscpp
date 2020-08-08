#pragma once

#include <filesystem>

namespace hscpp
{
    struct RuntimeDependency
    {
        enum class Type
        {
            Source,
            Include,
            Library,
        };

        Type type = {};
        std::vector<std::filesystem::path> paths;
    };
}