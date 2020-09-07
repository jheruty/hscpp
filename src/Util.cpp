#include <windows.h>
#include <algorithm>
#include <array>
#include <unordered_set>

#include "hscpp/Util.h"

namespace hscpp
{

    namespace util
    {

        const static std::unordered_set<std::string> HEADER_EXTENSIONS = {
            ".h",
            ".hh",
            ".hpp",
        };

        const static std::unordered_set<std::string> SOURCE_EXTENSIONS = {
            ".cpp",
            ".c",
            ".cc",
            ".cxx",
        };

        std::wstring GetErrorString(DWORD error)
        {
            if (error == ERROR_SUCCESS)
            {
                return L""; // No error.
            }

            LPWSTR buffer = nullptr;
            size_t size = FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<LPWSTR>(&buffer),
                0,
                NULL);

            std::wstring message(buffer, size);
            LocalFree(buffer);

            // Remove trailing '\r\n'. 
            if (message.size() >= 2
                && message.at(message.size() - 2) == '\r'
                && message.at(message.size() - 1) == '\n')
            {
                message.pop_back();
                message.pop_back();
            }

            return message;
        }

        std::wstring GetLastErrorString()
        {
            return GetErrorString(GetLastError());
        }

        bool IsWhitespace(const std::string& str)
        {
            return std::all_of(str.begin(), str.end(), ::isspace);
        }

        std::string Trim(const std::string& str)
        {
            std::string whitespace = " \t\n\v\f\r";

            size_t iFirst = str.find_first_not_of(whitespace);
            size_t iLast = str.find_last_not_of(whitespace);

            if (iFirst != std::string::npos && iLast != std::string::npos)
            {
                return str.substr(iFirst, iLast - iFirst + 1);
            }

            return "";
        }

        std::string CreateGuid()
        {
            GUID guid;
            CoCreateGuid(&guid);

            char buf[64];
            snprintf(buf, sizeof(buf), "%08X-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X",
                guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2],
                guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

            return buf;
        }

        bool IsHeaderFile(const fs::path& filePath)
        {
            fs::path extension = filePath.extension();
            return HEADER_EXTENSIONS.find(extension.string()) != HEADER_EXTENSIONS.end();
        }

        bool IsSourceFile(const fs::path& filePath)
        {
            fs::path extension = filePath.extension();
            return SOURCE_EXTENSIONS.find(extension.string()) != SOURCE_EXTENSIONS.end();
        }

    }

}