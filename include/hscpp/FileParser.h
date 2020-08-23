#pragma once

#include <filesystem>
#include <vector>

namespace hscpp
{

    namespace fs = std::filesystem;

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
            std::vector<fs::path> paths;
        };

        struct ParseInfo
        {
            std::vector<Require> requires;
            std::vector<std::string> preprocessorDefinitions;
            std::vector<std::string> modules;
            std::vector<fs::path> includes;
        };

        ParseInfo Parse(const fs::path& path);

    private:
        fs::path m_Filepath;

        size_t m_iChar = 0;
        std::string m_Content;

        void Parse(ParseInfo& info);
        bool ParseRequire(Require& require);
        bool ParsePreprocessorDefinitions(std::vector<std::string>& definitions);
        bool ParseModules(std::vector<std::string>& modules);
        bool ParseInclude(fs::path& include);

        bool ParseString(std::string& strContent, char startChar, char endChar);
        bool ParseIdentifier(std::string& identifier);

        bool Match(const std::string& str);
        bool Expect(char c, const std::string& error);
        void SkipWhitespace();
        void SkipComment();
        void SkipString();

        bool IsAlpha(char c);
        bool IsDigit(char c);

        bool IsAtEnd();
        char Peek();
        char PeekNext();
        void Advance();

        void LogParseError(const std::string& error);
    };

}

