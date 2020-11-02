#pragma once

#include <string>

#include "hscpp/preprocessor/Token.h"
#include "hscpp/Platform.h"

namespace hscpp
{

    class Lexer
    {
    public:
        bool Parse(const fs::path& filePath, std::vector<Token>& tokens);
        bool Parse(const std::string& content, std::vector<Token>& tokens);
        std::string GetLastError();

    private:
        std::string m_Content;
        size_t m_iChar = 0;
        size_t m_Column = 0;
        size_t m_Line = 1;

        std::string m_Error;

        void Reset(std::vector<Token>& tokens);
        bool Tokenize(std::vector<Token>& tokens);

        bool ParseString(char startChar, char endChar, std::vector<Token>& tokens);
        bool ParseIdentifier(std::vector<Token>& tokens);
        bool ParseNumber(std::vector<Token>& tokens);
        void PushToken(const std::string& value, Token::Type tokenType, std::vector<Token>& tokens);

        bool Match(const std::string& str);
        void SkipWhitespace();
        void SkipComment();

        bool IsAlpha(char c);
        bool IsDigit(char c);

        bool IsAtEnd();
        char Peek();
        char PeekNext();
        void Advance();

        void GenerateError(const std::string& error);
        void GenerateError(size_t iColumn, size_t iLine, const std::string& error);
    };

}