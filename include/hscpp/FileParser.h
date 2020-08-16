#pragma once

#include <filesystem>
#include <vector>

namespace hscpp
{

    class FileParser
    {
    public:
        struct Require
        {
            enum class Type
            {
                Source,
                Include,
                Library,
            };

            Type type = {};
            std::vector<std::filesystem::path> paths;
        };

        struct ParseInfo
        {
            std::vector<Require> requires;
        };

        ParseInfo Parse(const std::filesystem::path& path);

    private:
        std::filesystem::path m_Filepath;

        size_t m_iChar = 0;
        std::string m_Content;

        void Parse(std::vector<Require>& requires);
        bool ParseRequire(Require& require);

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

