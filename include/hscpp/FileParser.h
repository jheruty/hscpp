#pragma once

#include <filesystem>
#include <vector>
#include <functional>

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
            fs::path file;
            std::vector<Require> requires;
            std::vector<std::string> preprocessorDefinitions;
            std::vector<std::string> modules;
            std::vector<fs::path> includes;
        };

        ParseInfo Parse(const fs::path& file);

    private:
        fs::path m_Filepath;

        size_t m_iChar = 0;
        std::string m_Content;
        std::string m_Context;

        void Parse(ParseInfo& info);
        bool ParseRequire(Require& require);
        bool ParsePreprocessorDefinitions(std::vector<std::string>& definitions);
        bool ParseModules(std::vector<std::string>& modules);
        bool ParseInclude(fs::path& include);

        bool ParseArgumentList(const std::function<bool()>& parseArgumentCb);
        bool ParseString(char startChar, char endChar, std::string& strContent);
        bool ParseIdentifier(std::string& identifier);

        bool Match(const std::string& str);
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

