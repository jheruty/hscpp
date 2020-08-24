#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <filesystem>

namespace hscpp
{
    namespace fs = std::filesystem;

    namespace util
    {
        std::string GetErrorString(DWORD error);
        std::string GetLastErrorString();

        bool IsWhitespace(const std::string& str);
        std::string Trim(const std::string& str);

        std::string CreateGuid();

        bool IsHeaderFile(const fs::path& path);
        bool IsSourceFile(const fs::path& path);
    }
}