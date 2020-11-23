#include <fstream>
#include <sstream>
#include <thread>

#include "catch/catch.hpp"
#include "common/Util.h"
#include "common/Typedefs.h"

#include "hscpp/Platform.h"
#include "hscpp/Util.h"

namespace hscpp { namespace test
{

    fs::path CreateSandboxDirectory()
    {
        fs::path sandboxPath = util::GetHscppTestPath() / "sandbox";

        REQUIRE_NOTHROW(fs::remove_all(sandboxPath));
        REQUIRE(fs::create_directory(sandboxPath));

        return sandboxPath;
    }

    fs::path InitializeSandbox(const fs::path& assetsPath)
    {
        fs::path sandboxPath = CreateSandboxDirectory();

        std::error_code error;
        fs::copy(assetsPath, sandboxPath, fs::copy_options::overwrite_existing | fs::copy_options::recursive);
        REQUIRE(error.value() == HSCPP_ERROR_SUCCESS);

        // Wait a bit to ensure OS filesystem events settle down.
        std::this_thread::sleep_for(Milliseconds(50));

        return sandboxPath;
    }

    void NewFile(const fs::path& filePath, const std::string& content)
    {
        std::ofstream file(filePath.c_str());
        REQUIRE(file.is_open());

        file << content;
    }

    void ModifyFile(const fs::path& filePath, const std::unordered_map<std::string, std::string>& replacements)
    {
        std::string content = FileToString(filePath);

        // Replace contents within:
        // // <tag>
        // // </tag>
        // Where tag is the key in replacements, with the value in replacements.
        for (const auto& pattern__replacement : replacements)
        {
            std::string tagOpen = "// <" + pattern__replacement.first + ">";
            std::string tagClose = "// </" + pattern__replacement.first + ">";
            std::string replacement = "\n" + pattern__replacement.second + "\n";

            size_t iTagOpen = content.find(tagOpen);
            REQUIRE(iTagOpen != std::string::npos);

            size_t iTagClose = content.find(tagClose);
            REQUIRE(iTagClose != std::string::npos);

            content.replace(iTagOpen + tagOpen.size(), iTagClose - (iTagOpen + tagOpen.size()), replacement);
        }

        std::ofstream outFile(filePath.c_str());
        REQUIRE(outFile.is_open());

        outFile << content;
    }

    void RemoveFile(const fs::path& filePath)
    {
        std::error_code error;
        fs::remove(filePath, error);

        REQUIRE(error.value() == HSCPP_ERROR_SUCCESS);
    }

    void RenameFile(const fs::path& oldFilePath, const fs::path& newFilePath)
    {
        std::error_code error;
        fs::rename(oldFilePath, newFilePath, error);

        REQUIRE(error.value() == HSCPP_ERROR_SUCCESS);
    }

    std::string FileToString(const fs::path& filePath)
    {
        REQUIRE(fs::exists(filePath));

        std::ifstream ifs(filePath.u8string());
        REQUIRE(ifs.is_open());

        std::stringstream ss;
        ss << ifs.rdbuf();

        return ss.str();
    }

        fs::path Canonical(const fs::path& filePath)
    {
        std::error_code error;
        fs::path canonicalPath = fs::canonical(filePath);

        REQUIRE(error.value() == HSCPP_ERROR_SUCCESS);

        return canonicalPath;
    }

    void RunWin32(const std::function<void()>& cb)
    {
#if defined(HSCPP_PLATFORM_WIN32)
        cb();
#else
        HSCPP_UNUSED_PARAM(cb);
#endif
    }

    void RunUnix(const std::function<void()>& cb)
    {
#if defined(HSCPP_PLATFORM_UNIX)
        cb();
#else
        HSCPP_UNUSED_PARAM(cb);
#endif
    }

}}