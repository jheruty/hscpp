#pragma once

#include <Windows.h>
#include <string>
#include <vector>

namespace hscpp
{
    std::string GetErrorString(DWORD error);
    std::string GetLastErrorString();

    bool IsWhitespace(const std::string& str);
    std::string Trim(const std::string& str);

    std::string CreateGuid();
}