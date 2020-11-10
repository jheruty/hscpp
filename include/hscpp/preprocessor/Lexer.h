#pragma once

#include <string>

#include "hscpp/preprocessor/Token.h"
#include "hscpp/preprocessor/LangError.h"
#include "hscpp/Platform.h"

namespace hscpp
{

    class Lexer
    {
    public:
        bool Lex(const std::string& content, std::vector<Token>& tokens);
        LangError GetLastError();

    private:
        std::string m_Content;
        size_t m_iChar = 0;
        size_t m_Column = 0;
        size_t m_Line = 1;

        std::vector<Token>* m_pTokens = nullptr;

        LangError m_Error = LangError(LangError::Code::Success);

        void Reset(const std::string& content, std::vector<Token>& tokens);
        bool Lex();

        void LexString(char endChar);
        void LexIdentifier();
        void LexNumber();
        void PushToken(const std::string& value, Token::Type tokenType);

        bool Match(const std::string& str);
        void SkipWhitespace();
        void SkipComment();

        bool IsAlpha(char c);
        bool IsDigit(char c);

        bool IsAtEnd();
        char Peek();
        char PeekNext();
        void Advance();

        void ThrowError(const LangError& error);
    };

}