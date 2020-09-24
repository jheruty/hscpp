#include <algorithm>
#include <array>
#include <unordered_set>

#include "hscpp/Util.h"
#include "hscpp/Log.h"
#include "hscpp/FsPathHasher.h"

#if defined(HSCPP_PLATFORM_WIN32)

#include <Windows.h>

#elif defined(HSCPP_PLATFORM_UNIX)

#include <uuid/uuid.h>

#endif

namespace hscpp { namespace util
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

#if defined(HSCPP_PLATFORM_WIN32)

    std::wstring GetErrorString(TOsError error)
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

#elif defined(HSCPP_PLATFORM_UNIX)

    std::wstring GetErrorString(TOsError error)
    {
        std::string errorStr = strerror(errno);
        return std::wstring(errorStr.begin(), errorStr.end());
    }

    std::wstring GetLastErrorString()
    {
        return GetErrorString(errno);
    }

#endif

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

#if defined(HSCPP_PLATFORM_WIN32)

        GUID guid;
        CoCreateGuid(&guid);

        char buf[64];
        snprintf(buf, sizeof(buf), "%08X-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X",
            guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2],
            guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

        return buf;

#elif defined(HSCPP_PLATFORM_UNIX)

        uuid_t uuid;
        uuid_generate_random(uuid);

        char buf[64];
        uuid_unparse(uuid, buf);

        return buf;
#endif

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

    void SortFileEvents(const std::vector<IFileWatcher::Event>& events,
                        std::vector<fs::path>& canonicalModifiedFilePaths,
                        std::vector<fs::path>& canonicalRemovedFilePaths)
    {
        canonicalModifiedFilePaths.clear();
        canonicalRemovedFilePaths.clear();

        // A file may have duplicate events, compress these into a single event.
        std::unordered_set<fs::path, FsPathHasher> dedupedModifiedFilePaths;
        std::unordered_set<fs::path, FsPathHasher> dedupedRemovedFilePaths;

        for (const auto& event : events)
        {
            // If the file was removed, we cannot get its canonical filename through the relative
            // filename. However, we can get the canonical path of its parent directory, and use
            // that to construct the canonical path of the file.
            std::error_code error;
            fs::path directoryPath = event.filePath.parent_path();
            fs::path canonicalDirectoryPath = fs::canonical(directoryPath, error);

            if (error.value() == HSCPP_ERROR_FILE_NOT_FOUND)
            {
                // Entire directory was removed. Hscpp does not support this use case.
                log::Warning() << "Directory " << directoryPath << " was removed; hscpp does not support "
                    << "removing directories at runtime." << log::End();
                continue;
            }

            // Construct the canonical path of the file. Note that this also works for deleted files.
            fs::path canonicalFilePath = canonicalDirectoryPath / event.filePath.filename();

            if (fs::exists(canonicalFilePath))
            {
                // Had a file event and the file exists; it must have been added or modified.
                dedupedModifiedFilePaths.insert(canonicalFilePath);
            }
            else
            {
                // Had a file event and the file no longer exists; it must have been deleted.
                dedupedRemovedFilePaths.insert(canonicalFilePath);
            }
        }

        canonicalModifiedFilePaths = std::vector(dedupedModifiedFilePaths.begin(), dedupedModifiedFilePaths.end());
        canonicalRemovedFilePaths = std::vector(dedupedRemovedFilePaths.begin(), dedupedRemovedFilePaths.end());
    }

}}