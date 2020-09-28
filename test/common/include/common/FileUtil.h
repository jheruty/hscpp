#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "hscpp/Filesystem.h"

namespace hscpp { namespace test
{

    fs::path CreateSandboxDirectory();

    fs::path InitializeSandbox(const fs::path& assetsPath);

    void NewFile(const fs::path& filePath, const std::string& content);
    void ModifyFile(const fs::path& filePath, const std::unordered_map<std::string, std::string>& replacements);
    void RemoveFile(const fs::path& filePath);
    fs::path Canonical(const fs::path& filePath);

}}