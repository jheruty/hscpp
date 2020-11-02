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
        { "hscpp_if", Token::Type::HscppIf },
        { "hscpp_elif", Token::Type::HscppElif },
        { "hscpp_else", Token::Type::HscppElse },
        { "hscpp_end", Token::Type::HscppEnd },
    };

    bool Lexer::Parse(const fs::path& filePath, std::vector<Token>& tokens)
    {
        std::ifstream file(filePath.native().c_str());
        if (!file.is_open())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to open file " << filePath << log::End(".");
            return false;
        }

        std::stringstream buf;
        buf << file.rdbuf();

        return Parse(buf.str(), tokens);
    }

    bool Lexer::Parse(const std::string& content, std::vector<Token>& tokens)
    {
        Reset(tokens);

        m_Content = content;
        return Tokenize(tokens);
    }

    std::string Lexer::GetLastError()
    {
        return m_Error;
    }

    void Lexer::Reset(std::vector<Token>& tokens)
    {
        tokens.clear();

        m_Content.clear();
        m_iChar = 0;
        m_Column = 0;
        m_Line = 1;

        m_Error.clear();
    }

    bool Lexer::Tokenize(std::vector<Token>& tokens)
    {
        while (!IsAtEnd())
        {
            switch (Peek())
            {
                case '(':
                    Advance();
                    PushToken("(", Token::Type::LeftParen, tokens);
                    break;
                case ')':
                    Advance();
                    PushToken(")", Token::Type::RightParen, tokens);
                    break;
                case ',':
                    Advance();
                    PushToken(",", Token::Type::Comma, tokens);
                    break;
                case '=':
                    Advance();
                    if (Peek() == '=')
                    {
                        Advance();
                        PushToken("==", Token::Type::Equivalent, tokens);
                    }
                    break;
                case '<':
                    Advance();
                    if (Peek() == '=')
                    {
                        Advance();
                        PushToken("<=", Token::Type::LessThanOrEqual, tokens);
                    }
                    else
                    {
                        PushToken("<", Token::Type::LessThan, tokens);
                    }
                    break;
                case '>':
                    Advance();
                    if (Peek() == '=')
                    {
                        Advance();
                        PushToken(">=", Token::Type::GreaterThanOrEqual, tokens);
                    }
                    else
                    {
                        PushToken(">", Token::Type::GreaterThan, tokens);
                    }
                    break;
                case '&':
                    Advance();
                    if (Peek() == '&')
                    {
                        Advance();
                        PushToken("&&", Token::Type::LogicalAnd, tokens);
                    }
                    break;
                case '|':
                    Advance();
                    if (Peek() == '|')
                    {
                        Advance();
                        PushToken("||", Token::Type::LogicalOr, tokens);
                    }
                    break;
                case '\n':
                    Advance();
                    m_Line++;
                    m_Column = 0;
                    break;
                case '/':
                    SkipComment();
                    break;
                case '"':
                    ParseString('"', '"', tokens);
                    break;
                case '#':
                    Advance();
                    SkipWhitespace(); // # include is valid syntax.
                    if (Match("include"))
                    {
                        PushToken("#include", Token::Type::Include, tokens);

                        // Treat includes in < > as a string.
                        SkipWhitespace();
                        if (Peek() == '<')
                        {
                            ParseString('<', '>', tokens);
                        }
                    }
                    break;
                default:
                    if (IsAlpha(Peek()) || Peek() == '_')
                    {
                        ParseIdentifier(tokens);
                    }
                    else if (IsDigit(Peek()))
                    {
                        ParseNumber(tokens);
                    }
                    else
                    {
                        Advance();
                    }
            }

            if (!m_Error.empty())
            {
                return false;
            }
        }

        return true;
    }

    bool Lexer::ParseIdentifier(std::vector<Token>& tokens)
    {
        std::string identifier;

        if (IsAlpha(Peek()) || Peek() == '_')
        {
            while (IsAlpha(Peek()) || IsDigit(Peek()) || Peek() == '_')
            {
                identifier += Peek();
                Advance();
            }

            auto keywordIt = KEYWORDS.find(identifier);
            if (keywordIt != KEYWORDS.end())
            {
                PushToken(identifier, keywordIt->second, tokens);
            }
            else
            {
                PushToken(identifier, Token::Type::Identifier, tokens);
            }

            return true;
        }
        else
        {
            return false;
        }
    }

    bool Lexer::ParseNumber(std::vector<Token>& tokens)
    {
        std::string number;

        if (IsDigit(Peek()))
        {
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

            PushToken(number, Token::Type::Number, tokens);
            return true;
        }

        return false;
    }

    void Lexer::PushToken(const std::string& value, Token::Type tokenType, std::vector<Token>& tokens)
    {
        Token token;
        token.value = value;
        token.type = tokenType;
        token.line = m_Line;
        token.column = m_Column;

        tokens.push_back(token);
    }

    bool Lexer::ParseString(char startChar, char endChar, std::vector<Token>& tokens)
    {
        std::string str;

        if (Peek() != startChar)
        {
            std::string error = "Expected a '\"' but saw '" + std::string(1, Peek()) + "'.";
            GenerateError(error);

            return false;
        }

        Advance();

        while (!IsAtEnd() && Peek() != endChar)
        {
            if (Peek() == '\\' && PeekNext() == '"')
            {
                // Escaped quote.
                Advance();
                Advance();
                str += '"';
            }
            else
            {
                str += Peek();
                Advance();
            }
        }

        if (Peek() != endChar)
        {
            std::string error = std::string("Unterminated string, expected a ") + endChar + ".";
            GenerateError(error);

            return false;
        }

        Advance();
        PushToken(str, Token::Type::String, tokens);

        return true;
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
            ++m_iChar;
            ++m_Column;
        }
    }

    void Lexer::GenerateError(const std::string& error)
    {
        GenerateError(m_Column, m_Line, error);
    }

    void Lexer::GenerateError(size_t iColumn, size_t iLine, const std::string& error)
    {
        m_Error = error + "(Line: " + std::to_string(iLine) + ", Column: " + std::to_string(iColumn) + ")";
    }

}