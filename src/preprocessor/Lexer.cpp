#include <fstream>
#include <unordered_map>

#include "hscpp/preprocessor/Lexer.h"
#include "hscpp/Log.h"

namespace hscpp
{

    const static std::unordered_map<std::string, Token::Type> KEYWORDS = {
        { "hscpp_require_source", Token::Type::HscppRequireSource },
        { "hscpp_require_include_dir", Token::Type::HscppRequireIncludeDir },
        { "hscpp_require_library", Token::Type::HscppRequireLibrary },
        { "hscpp_require_library_dir", Token::Type::HscppRequireLibraryDir },
        { "hscpp_require_preprocessor_def", Token::Type::HscppRequirePreprocessorDef },
        { "hscpp_module", Token::Type::HscppModule },
        { "hscpp_message", Token::Type::HscppMessage },
        { "hscpp_if", Token::Type::HscppIf },
        { "hscpp_elif", Token::Type::HscppElif },
        { "hscpp_else", Token::Type::HscppElse },
        { "hscpp_end", Token::Type::HscppEnd },
        { "hscpp_return", Token::Type::HscppReturn },
        { "HSCPP_TRACK", Token::Type::HscppTrack },
        { "true", Token::Type::Bool },
        { "false", Token::Type::Bool },
    };

    bool Lexer::Lex(const std::string& content, std::vector<Token>& tokens)
    {
        Reset(content, tokens);

        try
        {
            return Lex();
        }
        catch (const std::runtime_error&)
        {
            return false;
        }
    }

    LangError Lexer::GetLastError()
    {
        return m_Error;
    }

    void Lexer::Reset(const std::string& content, std::vector<Token>& tokens)
    {
        m_Content = content;
        m_iChar = 0;
        m_Column = 0;
        m_Line = 1;

        tokens.clear();
        m_pTokens = &tokens;

        m_Error = LangError(LangError::Code::Success);
    }

    bool Lexer::Lex()
    {
        while (!IsAtEnd())
        {
            size_t iStartChar = m_iChar;

            switch (Peek())
            {
                case '(':
                    Advance();
                    PushToken("(", Token::Type::LeftParen);
                    break;
                case ')':
                    Advance();
                    PushToken(")", Token::Type::RightParen);
                    break;
                case ',':
                    Advance();
                    PushToken(",", Token::Type::Comma);
                    break;
                case '=':
                    if (PeekNext() == '=')
                    {
                        Advance();
                        Advance();
                        PushToken("==", Token::Type::Equivalent);
                    }
                    else
                    {
                        Advance();
                        PushToken("=", Token::Type::Unknown);
                    }
                    break;
                case '!':
                    if (PeekNext() == '=')
                    {
                        Advance();
                        Advance();
                        PushToken("!=", Token::Type::Inequivalent);
                    }
                    else
                    {
                        Advance();
                        PushToken("!", Token::Type::Exclamation);
                    }
                    break;
                case '<':
                    if (PeekNext() == '=')
                    {
                        Advance();
                        Advance();
                        PushToken("<=", Token::Type::LessThanOrEqual);
                    }
                    else
                    {
                        Advance();
                        PushToken("<", Token::Type::LessThan);
                    }
                    break;
                case '>':
                    if (PeekNext() == '=')
                    {
                        Advance();
                        Advance();
                        PushToken(">=", Token::Type::GreaterThanOrEqual);
                    }
                    else
                    {
                        Advance();
                        PushToken(">", Token::Type::GreaterThan);
                    }
                    break;
                case '&':
                    if (PeekNext() == '&')
                    {
                        Advance();
                        Advance();
                        PushToken("&&", Token::Type::LogicalAnd);
                    }
                    else
                    {
                        Advance();
                        PushToken("&", Token::Type::Unknown);
                    }
                    break;
                case '|':
                    if (PeekNext() == '|')
                    {
                        Advance();
                        Advance();
                        PushToken("||", Token::Type::LogicalOr);
                    }
                    else
                    {
                        Advance();
                        PushToken("|", Token::Type::Unknown);
                    }
                    break;
                case '+':
                    Advance();
                    PushToken("+", Token::Type::Plus);
                    break;
                case '-':
                    Advance();
                    PushToken("-", Token::Type::Minus);
                    break;
                case '/':
                    if (PeekNext() == '/' || PeekNext() == '*')
                    {
                        SkipComment();
                    }
                    else
                    {
                        Advance();
                        PushToken("/", Token::Type::Slash);
                    }
                    break;
                case '*':
                    Advance();
                    PushToken("*", Token::Type::Star);
                    break;
                case '"':
                    LexString('"');
                    break;
                case '#':
                    Advance();
                    SkipWhitespace(); // # include is valid syntax.
                    if (Match("include"))
                    {
                        PushToken("#include", Token::Type::Include);

                        // Treat includes with < > as a string.
                        SkipWhitespace();
                        if (Peek() == '<')
                        {
                            LexString('>');
                        }
                    }
                    break;
                case ' ':
                case '\t':
                case '\n':
                case '\v':
                case '\f':
                case '\r':
                    SkipWhitespace();
                    break;
                default:
                    if (IsAlpha(Peek()) || Peek() == '_')
                    {
                        LexIdentifier();
                    }
                    else if (IsDigit(Peek()))
                    {
                        LexNumber();
                    }
                    else
                    {
                        PushToken(std::string(1, Peek()), Token::Type::Unknown);
                        Advance();
                    }
            }

            if (m_iChar == iStartChar)
            {
                // The above switch statement should cause the lexer to advance for any input.
                // Just in case, check if a manual advancement is needed, to avoid an infinite
                // loop due to a Lexer bug.
                log::Warning() << HSCPP_LOG_PREFIX
                    << "Lexer failed to advance. Forcing advancement." << log::End();
                Advance();
            }

        }

        return true;
    }

    void Lexer::LexString(char endChar)
    {
        size_t startLine = m_Line;
        std::string str;

        Advance(); // Skip opening '"' or '<'.

        while (!IsAtEnd() && Peek() != endChar)
        {
            if (Peek() == '\\' && PeekNext() == '"')
            {
                // Escaped quote.
                Advance();
                Advance();
                str += '"';
            }
            else if (Peek() == '\\' && PeekNext() == '\\')
            {
                // Escaped slash.
                Advance();
                Advance();
                str += '\\';
            }
            else
            {
                // Not handling other escape sequences (ex. \n).
                str += Peek();
                Advance();
            }
        }

        if (Peek() != endChar)
        {
            // Note that error includes start line and beginning of string, to make it easier to
            // determine the problematic string (m_Line and m_Column will point to end of file).
            ThrowError(LangError(LangError::Code::Lexer_UnterminatedString,
                    m_Line, m_Column, { std::string(1, endChar), std::to_string(startLine), str.substr(0, 10) }));
        }

        Advance();
        PushToken(str, Token::Type::String);
    }

    void Lexer::LexIdentifier()
    {
        std::string identifier;

        while (IsAlpha(Peek()) || IsDigit(Peek()) || Peek() == '_')
        {
            identifier += Peek();
            Advance();
        }

        auto keywordIt = KEYWORDS.find(identifier);
        if (keywordIt != KEYWORDS.end())
        {
            PushToken(identifier, keywordIt->second);
        }
        else
        {
            PushToken(identifier, Token::Type::Identifier);
        }
    }

    void Lexer::LexNumber()
    {
        std::string number;

        while (IsDigit(Peek()))
        {
            number += Peek();
            Advance();
        }

        if (Peek() == '.')
        {
            number += ".";
            Advance();

            while (IsDigit(Peek()))
            {
                number += Peek();
                Advance();
            }
        }

        PushToken(number, Token::Type::Number);
    }

    void Lexer::PushToken(const std::string& value, Token::Type tokenType)
    {
        Token token;
        token.value = value;
        token.type = tokenType;
        token.line = m_Line;
        token.column = m_Column;

        m_pTokens->push_back(token);
    }

    bool Lexer::Match(const std::string& str)
    {
        size_t iChar = m_iChar;
        size_t iOffset = 0;

        while (iChar < m_Content.size() && iOffset < str.size())
        {
            if (str.at(iOffset) != m_Content.at(iChar))
            {
                return false;
            }

            ++iChar;
            ++iOffset;
        }

        if (iOffset != str.size())
        {
            // Hit end of content before matching str.
            return false;
        }

        // Success, consume the match.
        m_iChar = iChar;
        return true;
    }

    void Lexer::SkipWhitespace()
    {
        while (!IsAtEnd() && std::isspace(Peek()))
        {
            Advance();
        }
    }

    void Lexer::SkipComment()
    {
        if (Peek() == '/')
        {
            if (PeekNext() == '/')
            {
                Advance(); // /
                Advance(); // /
                while (!IsAtEnd() && Peek() != '\n')
                {
                    Advance();
                }
                Advance(); // \n
            }
            else if (PeekNext() == '*')
            {
                Advance(); // /
                Advance(); // *
                while (!IsAtEnd())
                {
                    if (Peek() == '*' && PeekNext() == '/')
                    {
                        break;
                    }
                    else
                    {
                        Advance();
                    }
                }
                Advance(); // *
                Advance(); // /
            }
        }
    }

    bool Lexer::IsAlpha(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    bool Lexer::IsDigit(char c)
    {
        return c >= '0' && c <= '9';
    }

    bool Lexer::IsAtEnd()
    {
        return m_iChar >= m_Content.size();
    }

    char Lexer::Peek()
    {
        if (IsAtEnd())
        {
            return 0;
        }

        return m_Content.at(m_iChar);
    }

    char Lexer::PeekNext()
    {
        if (m_iChar + 1 >= m_Content.size())
        {
            return 0;
        }

        return m_Content.at(m_iChar + 1);
    }

    void Lexer::Advance()
    {
        if (!IsAtEnd())
        {
            if (Peek() == '\n')
            {
                ++m_Line;
                m_Column = 0;
            }
            else
            {
                ++m_Column;
            }

            ++m_iChar;
        }
    }

    void Lexer::ThrowError(const LangError& error)
    {
        m_Error = error;
        throw std::runtime_error("");
    }

}