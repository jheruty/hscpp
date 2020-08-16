#pragma once

#include <filesystem>
#include <vector>

#include "hscpp/RuntimeDependency.h"

namespace hscpp
{

    class FileParser
    {
    public:
        bool ParseDependencies(const std::filesystem::path& path, std::vector<RuntimeDependency>& dependencies);

    private:
        std::filesystem::path m_Filepath;

        size_t m_iChar = 0;
        std::string m_Content;

        void ParseDependencies(std::vector<RuntimeDependency>& info);
        bool ParseDependency(RuntimeDependency& dependency);

        bool ParseString(std::string& strContent);

        bool Match(const std::string& str);
        bool Expect(char c, const std::string& error);
        void SkipWhitespace();
        void SkipComment();
        void SkipString();

        bool IsAtEnd();
        char Peek();
        char PeekNext();
        void Advance();

        void LogParseError(const std::string& error);
    };

}

