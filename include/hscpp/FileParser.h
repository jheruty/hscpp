#pragma once

#include <filesystem>
#include <vector>

#include "hscpp/RuntimeDependency.h"

namespace hscpp
{

    class FileParser
    {
    public:
        struct ParseInfo
        {
            std::vector<RuntimeDependency> dependencies;
            std::vector<std::filesystem::path> includes;
        };

        bool ParseFile(const std::filesystem::path& path, ParseInfo& info);

    private:
        std::filesystem::path m_Filepath;

        size_t m_iChar = 0;
        std::string m_Content;

        void Parse(ParseInfo& info);
        bool ParseRuntimeRequirement(RuntimeDependency& dependency);
        bool ParseInclude(std::filesystem::path& include);

        bool ParseString(std::string& strContent);

        bool Match(const std::string& str);
        bool Expect(char c, const std::string& error);
        void SkipWhitespace();

        bool IsAtEnd();
        char Peek();
        void Advance();

        void LogParseError(const std::string& error);
    };

}

