#include <fstream>
#include <sstream>
#include <thread>

#include "catch/catch.hpp"
#include "common/FileUtil.h"
#include "common/Typedefs.h"
#include "hscpp/Platform.h"

namespace hscpp { namespace test
{

    fs::path RootTestDirectory()
    {
        return fs::path(__FILE__).parent_path() / ".." / "..";
    }

    fs::path CreateSandboxDirectory()
    {
        fs::path rootDirectoryPath = RootTestDirectory();
        fs::path sandboxPath = rootDirectoryPath / "sandbox";

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
        REQUIRE(fs::exists(filePath));

        std::ifstream inFile(filePath.c_str());
        REQUIRE(inFile.is_open());

        std::stringstream buf;
        buf << inFile.rdbuf();

        std::string content = buf.str();

        // Replace all (ex.) ${Body} with the values of the key Body in replacements.
        for (const auto& pattern__replacement : replacements)
        {
            std::string pattern = "${" + pattern__replacement.first + "}";
            std::string replacement = pattern__replacement.second;

            size_t iPos = 0;
            do
            {
                iPos = content.find(pattern, iPos);
                if (iPos != std::string::npos)
                {
                    content.replace(iPos, pattern.size(), replacement);
                }
            } while (iPos != std::string::npos);
        }

        // Close file, we are going to open it again for writing.
        inFile.close();

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

    fs::path Canonical(const fs::path &filePath)
    {
        std::error_code error;
        fs::path canonicalPath = fs::canonical(filePath);

        REQUIRE(error.value() == HSCPP_ERROR_SUCCESS);

        return canonicalPath;
    }

}}