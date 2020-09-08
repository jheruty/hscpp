#pragma once

#include <filesystem>

namespace hscpp
{

    // Conceptually, this is part of the platform. Need to put in its own file to avoid circular
    // dependency between interfaces (ex. ICompiler) and Platform.h
    namespace fs = std::filesystem;

}