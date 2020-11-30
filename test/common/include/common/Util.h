#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include "hscpp/Filesystem.h"

namespace hscpp { namespace test
{

    fs::path CreateSandboxDirectory();
    fs::path InitializeSandbox(const fs::path& assetsPath);

    void NewFile(const fs::path& filePath, const std::string& content);
    void ModifyFile(const fs::path& filePath, const std::unordered_map<std::string, std::string>& replacements);
    void RemoveFile(const fs::path& filePath);
    void RenameFile(const fs::path& oldFilePath, const fs::path& newFilePath);
    std::string FileToString(const fs::path& filePath);
    fs::path Canonical(const fs::path& filePath);

    void RunWin32(const std::function<void()>& cb);
    void RunUnix(const std::function<void()>& cb);

}}