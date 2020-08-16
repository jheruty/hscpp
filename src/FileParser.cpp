#include <fstream>
#include <sstream>

#include "hscpp/FileParser.h"
#include "hscpp/Log.h"

namespace hscpp
{

    FileParser::ParseInfo FileParser::Parse(const std::filesystem::path& path)
    {
        ParseInfo info;

        std::ifstream file(path.native().c_str());
        if (!file.is_open())
        {
            Log::Write(LogLevel::Error, "%s: Failed to open file %s.\n", __func__, path.u8string().c_str());
            return info;
        }

        std::stringstream buf;
        buf << file.rdbuf();

        m_Filepath = path;
        m_iChar = 0;
        m_Content = buf.str();

        Parse(info.requires);

        return info;
    }

    void FileParser::Parse(std::vector<Require>& requires)
    {
        // Barebones lexer/parser. There are very few things we need to match, so a complex parser
        // is not needed.
        while (!IsAtEnd())
        {
            size_t iStartChar = m_iChar;

            // We only care to find:
            //    hscpp_require_source
            //    hscpp_require_include
            //    hscpp_require_lib
            //    #include
            switch (Peek())
            {
            case '/':
                SkipComment();
                break;
            case '"':
                SkipString();
                break;
            case 'h':
            {
                bool bRequire = false;
                Require require;

                if (Match("hscpp_require_"))
                {
                    if (Match("source"))
                    {
                        bRequire = true;
                        require.type = Require::Type::Source;
                    }
                    else if (Match("include"))
                    {
                        bRequire = true;
                        require.type = Require::Type::Include;
                    }
                    else if (Match("lib"))
                    {
                        bRequire = true;
                        require.type = Require::Type::Library;
                    }

                    if (bRequire)
                    {
                        if (ParseRequire(require))
                        {
                            requires.push_back(require);
                        }
                    }
                }

                break;
            }
            }

            if (iStartChar == m_iChar)
            {
                Advance();
            }
        }
    }

    bool FileParser::ParseRequire(Require& require)
    {
        SkipWhitespace();      

        if (Peek() != '(')
        {
            // Not a true error, in case user defined something like hscpp_require_source_custom.
            return false;
        }

        do
        {
            // Parse argument list (ex. hscpp_require_source(file1.cpp, file2.cpp) ).
            Advance();
            SkipWhitespace();

            std::string path;
            if (!ParseString(path))
            {
                return false;
            }

            require.paths.push_back(path);

            SkipWhitespace();
        } while (Peek() == ',');

        if (Peek() != ')')
        {
            LogParseError("Runtime dependency missing closing ')'.");
            return false;
        }

        Advance();
        return true;
    }

    bool FileParser::ParseString(std::string& strContent)
    {
        if (Peek() != '"')
        {
            LogParseError("Missing opening '\"'.");
            return false;
        }
        Advance();

        while (!IsAtEnd() && Peek() != '"')
        {
            if (Peek() == '\\' && PeekNext() == '"')
            {
                // Escaped quote.
                Advance();
                Advance();
                strContent += '"';
            }
            else
            {
                strContent += Peek();
                Advance();
            }
        }

        if (Peek() != '"')
        {
            LogParseError("Unterminated string, expected a '\"'.");
            return false;
        }

        Advance();
        return true;
    }

    bool FileParser::Match(const std::string& str)
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

    bool FileParser::Expect(char c, const std::string& error)
    {
        if (Peek() == c)
        {
            return true;
        }

        Log::Write(LogLevel::Error, "%s\n", error.c_str());
        return false;
    }

    void FileParser::SkipWhitespace()
    {
        while (!IsAtEnd() && std::isspace(Peek()))
        {
            Advance();
        }
    }

    void FileParser::SkipComment()
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
                while (!IsAtEnd() && Peek() != '*' && PeekNext() != '/')
                {
                    Advance();
                }
                Advance(); // *
                Advance(); // /
            }
        }
    }

    void FileParser::SkipString()
    {
        if (Peek() == '"')
        {
            Advance();
        }

        while (!IsAtEnd() && Peek() != '"')
        {
            if (Peek() == '\\' && PeekNext() == '"')
            {
                // Escaped quote.
                Advance();
                Advance();
            }
            else
            {
                Advance();
            }
        }

        Advance();
    }

    bool FileParser::IsAtEnd()
    {
        return m_iChar >= m_Content.size();
    }

    char FileParser::Peek()
    {
        if (IsAtEnd())
        {
            return 0;
        }

        return m_Content.at(m_iChar);
    }

    char FileParser::PeekNext()
    {
        if (m_iChar + 1 >= m_Content.size())
        {
            return 0;
        }

        return m_Content.at(m_iChar + 1);
    }

    void FileParser::Advance()
    {
        if (!IsAtEnd())
        {
            ++m_iChar;
        }
    }

    void FileParser::LogParseError(const std::string& error)
    {
        Log::Write(LogLevel::Error, "%s: Failed to parse file %s: %s\n",
            __func__, m_Filepath.u8string().c_str(), error.c_str());
    }

}
