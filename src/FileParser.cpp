#include <fstream>
#include <sstream>

#include "hscpp/FileParser.h"
#include "hscpp/Log.h"

namespace hscpp
{

    FileParser::ParseInfo FileParser::Parse(const fs::path& filePath)
    {
        ParseInfo info;
        info.filePath = filePath;

        std::ifstream file(filePath.native().c_str());
        if (!file.is_open())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to open file " << filePath << log::End(".");
            return info;
        }

        std::stringstream buf;
        buf << file.rdbuf();

        m_Content = buf.str();
        m_FilePath = filePath;
        m_iChar = 0;

        Parse(info);
        return info;
    }

    void FileParser::Parse(ParseInfo& info)
    {
        // Barebones lexer/parser. There are very few things we need to match, so a complex parser
        // is not needed.
        while (!IsAtEnd())
        {
            size_t iStartChar = m_iChar;
            m_Context.clear();

            // We only care to find:
            //    hscpp_require_source
            //    hscpp_require_include
            //    hscpp_require_lib
            //    hscpp_preprocessor_definitions
            //    hscpp_module 
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
                        m_Context = "hscpp_require_source";

                        bRequire = true;
                        require.type = Require::Type::Source;
                    }
                    else if (Match("include"))
                    {
                        m_Context = "hscpp_require_include";

                        bRequire = true;
                        require.type = Require::Type::Include;
                    }
                    else if (Match("lib"))
                    {
                        m_Context = "hscpp_require_lib";

                        bRequire = true;
                        require.type = Require::Type::Library;
                    }

                    if (bRequire)
                    {
                        if (ParseRequire(require))
                        {
                            info.requires.push_back(require);
                        }
                    }
                }
                else if (Match("hscpp_preprocessor_definitions"))
                {
                    m_Context = "hscpp_preprocessor_definitions";

                    std::vector<std::string> definitions;
                    if (ParsePreprocessorDefinitions(definitions))
                    {
                        info.preprocessorDefinitions.insert(info.preprocessorDefinitions.end(),
                            definitions.begin(), definitions.end());
                    }
                }
                else if (Match("hscpp_module"))
                {
                    m_Context = "hscpp_module";

                    std::vector<std::string> modules;
                    if (ParseModules(modules))
                    {
                        info.modules.insert(info.modules.end(), modules.begin(), modules.end());
                    }
                }

                break;
            }
            case '#':
            {
                Advance();
                SkipWhitespace(); // # include is valid syntax.

                if (Match("include"))
                {
                    m_Context = "#include";

                    fs::path includePath;
                    if (ParseInclude(includePath))
                    {
                        info.includePaths.push_back(includePath);
                    }
                }

                break;
            }
            }

            if (m_iChar == iStartChar)
            {
                Advance();
            }
        }
    }

    bool FileParser::ParseRequire(Require& require)
    {
        return ParseArgumentList([&]() {
            std::string path;
            if (!ParseString('"', '"', path))
            {
                return false;
            }

            require.paths.push_back(path);
            return true;
        });
    }

    bool FileParser::ParsePreprocessorDefinitions(std::vector<std::string>& definitions)
    {
        return ParseArgumentList([&]() {
            // Preprocessor definitions are accepted as both strings and literal identifiers.
            std::string definition;
            if (Peek() == '"')
            {
                if (!ParseString('"', '"', definition))
                {
                    return false;
                }
            }
            else
            {
                if (!ParseIdentifier(definition))
                {
                    return false;
                }
            }

            definitions.push_back(definition);
            return true;
        });
    }

    bool FileParser::ParseModules(std::vector<std::string>& modules)
    {
        return ParseArgumentList([&]() {
            std::string module;
            if (!ParseString('"', '"', module))
            {
                return false;
            }

            modules.push_back(module);
            return true;
        });
    }

    bool FileParser::ParseInclude(fs::path& includePath)
    {
        SkipWhitespace();

        std::string includeStr;
        if (Peek() == '"')
        {
            if (!ParseString('"', '"', includeStr))
            {
                return false;
            }
        }
        else if (Peek() == '<')
        {
            if (!ParseString('<', '>', includeStr))
            {
                return false;
            }
        }
        else
        {
            return false;
        }

        // Assume file is either ANSI or UTF-8, and read as UTF-8 to support Unicode include paths.
        includePath = fs::u8path(includeStr);
        return true;
    }

    bool FileParser::ParseArgumentList(const std::function<bool()>& parseArgumentCb)
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

            if (!parseArgumentCb())
            {
                // Do not report error, that is expected to be done in the parseArgumentCb.
                return false;
            }

            SkipWhitespace();
        } while (Peek() == ',');

        if (Peek() != ')')
        {
            std::string error = "Expected a ')' but saw '" + std::string(1, Peek()) + "'.";
            LogParseError(error);
            return false;
        }

        Advance();
        return true;
    }

    bool FileParser::ParseString(char startChar, char endChar, std::string& strContent)
    {
        if (Peek() != startChar)
        {
            std::string error = "Expected a '\"' but saw '" + std::string(1, Peek()) + "'.";
            LogParseError(error);
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
                strContent += '"';
            }
            else
            {
                strContent += Peek();
                Advance();
            }
        }

        if (Peek() != endChar)
        {
            std::string error = std::string("Unterminated string, expected a ") + endChar + ".";
            LogParseError(error);
            return false;
        }

        Advance();
        return true;
    }

    bool FileParser::ParseIdentifier(std::string& identifier)
    {
        if (IsAlpha(Peek()) || Peek() == '_')
        {
            while (IsAlpha(Peek()) || IsDigit(Peek()) || Peek() == '_')
            {
                identifier += Peek();
                Advance();
            }

            return true;
        }
        else
        {
            return false;
        }
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

    bool FileParser::IsAlpha(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    bool FileParser::IsDigit(char c)
    {
        return c >= '0' && c <= '9';
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
        log::Stream logStream = log::Error();
        logStream << HSCPP_LOG_PREFIX << "Parse error in file " << m_FilePath;

        if (!m_Context.empty())
        {
            logStream << "[" << m_Context << "]: ";
        }
        else
        {
            logStream << ": ";
        }

        logStream << error << log::End();
    }

}
