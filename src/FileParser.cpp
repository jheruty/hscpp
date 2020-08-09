#include <fstream>
#include <sstream>

#include "hscpp/FileParser.h"
#include "hscpp/Log.h"

namespace hscpp
{

    bool FileParser::ParseDependencies(const std::filesystem::path& path, std::vector<RuntimeDependency>& dependencies)
    {
        std::ifstream file(path.native().c_str());
        if (!file.is_open())
        {
            Log::Write(LogLevel::Error, "%s: Failed to open file %s.\n", __func__, path.u8string().c_str());
            return false;
        }

        std::stringstream buf;
        buf << file.rdbuf();

        m_Filepath = path;
        m_iChar = 0;
        m_Content = buf.str();

        ParseDependencies(dependencies);
        return true;
    }

    void FileParser::ParseDependencies(std::vector<RuntimeDependency>& dependencies)
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
                bool bDependency = false;
                RuntimeDependency::Type dependencyType = RuntimeDependency::Type::Source;

                if (Match("hscpp_require_"))
                {
                    if (Match("source"))
                    {
                        bDependency = true;
                        dependencyType = RuntimeDependency::Type::Source;
                    }
                    else if (Match("include"))
                    {
                        bDependency = true;
                        dependencyType = RuntimeDependency::Type::Include;
                    }
                    else if (Match("lib"))
                    {
                        bDependency = true;
                        dependencyType = RuntimeDependency::Type::Library;
                    }

                    if (bDependency)
                    {
                        RuntimeDependency dependency;
                        dependency.type = dependencyType;

                        if (ParseDependency(dependency))
                        {
                            dependencies.push_back(dependency);
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

    bool FileParser::ParseDependency(RuntimeDependency& dependency)
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

            dependency.paths.push_back(path);

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
                strContent += Advance();
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

    char FileParser::Advance()
    {
        m_iChar++;
        return m_Content.at(m_iChar - 1);
    }

    void FileParser::LogParseError(const std::string& error)
    {
        Log::Write(LogLevel::Error, "%s: Failed to parse file %s: %s\n",
            __func__, m_Filepath.u8string().c_str(), error.c_str());
    }

}
