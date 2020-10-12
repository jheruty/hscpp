#pragma once

// Conceptually, this is part of the platform. Need to put in its own file to avoid circular
// dependency between interfaces (ex. ICompiler) and Platform.h

#ifdef HSCPP_USE_GHC_FILESYSTEM

#include "ghc/filesystem.hpp"

namespace hscpp
{
    namespace fs = ghc::filesystem;
}

#else

#include <filesystem>

namespace hscpp
{
    namespace fs = std::filesystem;
}

#endif