#pragma once

#include <string>
#include <vector>

#include "hscpp/Platform.h"

namespace hscpp
{

    namespace util
    {
#ifdef HSCPP_PLATFORM_WIN32
        
        std::wstring GetErrorString(TOsError error);
        std::wstring GetLastErrorString();

#elif HSCPP_PLATFORM_UNIX

        std::wstring GetErrorString(TOsError error);
        std::wstring GetLastErrorString();

#endif

        bool IsWhitespace(const std::string& str);
        std::string Trim(const std::string& str);

        std::string CreateGuid();

        bool IsHeaderFile(const fs::path& filePath);
        bool IsSourceFile(const fs::path& filePath);
    }
}