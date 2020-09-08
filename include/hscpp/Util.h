#pragma once

#include <Windows.h>
#include <string>
#include <vector>

#include "hscpp/Platform.h"

namespace hscpp
{

    namespace util
    {
        std::wstring GetErrorString(DWORD error);
        std::wstring GetLastErrorString();

        bool IsWhitespace(const std::string& str);
        std::string Trim(const std::string& str);

        std::string CreateGuid();

        bool IsHeaderFile(const fs::path& filePath);
        bool IsSourceFile(const fs::path& filePath);
    }
}