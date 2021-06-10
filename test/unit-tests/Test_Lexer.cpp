#include "catch/catch.hpp"
#include "common/Common.h"
#include "hscpp/Platform.h"
#include "hscpp/Util.h"
#include "hscpp/preprocessor/Lexer.h"
#include "hscpp/preprocessor/LangError.h"

namespace hscpp { namespace test
{
    const static fs::path CLANG_FILES_PATH = util::GetHscppTestPath() / "unit-tests" / "files" / "clang-source";
    const static fs::path BOOST_FILE_PATH = util::GetHscppTestPath() / "unit-tests" / "files" / "boost-source";

    static void ValidateError(const std::string& program,
        const LangError::Code& expectedCode,
        size_t expectedLine,
        const std::vector<std::string>& expectedArgs)
    {
        std::vector<Token> tokens;
        Lexer lexer;

        REQUIRE_FALSE(lexer.Lex(program, tokens));
        CALL(ValidateError, lexer.GetLastError(), expectedCode, expectedLine, expectedArgs);
    }

    TEST_CASE("Lexer can lex all tokens.")
    {
        std::string str = R"(
            ( ) , == != < <= > >= && || + - / * ! _identifier0 1.0 "str str" #include <map>
            hscpp_require_source hscpp_require_include_dir /*comment*/ hscpp_require_library
            hscpp_require_library_dir hscpp_require_preprocessor_def //comment
            hscpp_module hscpp_if hscpp_elif hscpp_else hscpp_end hscpp_return

            HSCPP_TRACK

            true false

            = & |

            ^ // Unknown token.
        )";

        std::vector<Token> tokens;

        Lexer lexer;
        REQUIRE(lexer.Lex(str, tokens));

        int i = 0;

        REQUIRE(tokens.at(i).type == Token::Type::LeftParen);
        REQUIRE(tokens.at(i++).value == "(");
        REQUIRE(tokens.at(i).type == Token::Type::RightParen);
        REQUIRE(tokens.at(i++).value == ")");
        REQUIRE(tokens.at(i).type == Token::Type::Comma);
        REQUIRE(tokens.at(i++).value == ",");
        REQUIRE(tokens.at(i).type == Token::Type::Equivalent);
        REQUIRE(tokens.at(i++).value == "==");
        REQUIRE(tokens.at(i).type == Token::Type::Inequivalent);
        REQUIRE(tokens.at(i++).value == "!=");
        REQUIRE(tokens.at(i).type == Token::Type::LessThan);
        REQUIRE(tokens.at(i++).value == "<");
        REQUIRE(tokens.at(i).type == Token::Type::LessThanOrEqual);
        REQUIRE(tokens.at(i++).value == "<=");
        REQUIRE(tokens.at(i).type == Token::Type::GreaterThan);
        REQUIRE(tokens.at(i++).value == ">");
        REQUIRE(tokens.at(i).type == Token::Type::GreaterThanOrEqual);
        REQUIRE(tokens.at(i++).value == ">=");
        REQUIRE(tokens.at(i).type == Token::Type::LogicalAnd);
        REQUIRE(tokens.at(i++).value == "&&");
        REQUIRE(tokens.at(i).type == Token::Type::LogicalOr);
        REQUIRE(tokens.at(i++).value == "||");
        REQUIRE(tokens.at(i).type == Token::Type::Plus);
        REQUIRE(tokens.at(i++).value == "+");
        REQUIRE(tokens.at(i).type == Token::Type::Minus);
        REQUIRE(tokens.at(i++).value == "-");
        REQUIRE(tokens.at(i).type == Token::Type::Slash);
        REQUIRE(tokens.at(i++).value == "/");
        REQUIRE(tokens.at(i).type == Token::Type::Star);
        REQUIRE(tokens.at(i++).value == "*");
        REQUIRE(tokens.at(i).type == Token::Type::Exclamation);
        REQUIRE(tokens.at(i++).value == "!");
        REQUIRE(tokens.at(i).type == Token::Type::Identifier);
        REQUIRE(tokens.at(i++).value == "_identifier0");
        REQUIRE(tokens.at(i).type == Token::Type::Number);
        REQUIRE(tokens.at(i++).value == "1.0");
        REQUIRE(tokens.at(i).type == Token::Type::String);
        REQUIRE(tokens.at(i++).value == "str str");
        REQUIRE(tokens.at(i).type == Token::Type::Include);
        REQUIRE(tokens.at(i++).value == "#include");
        REQUIRE(tokens.at(i).type == Token::Type::String);
        REQUIRE(tokens.at(i++).value == "map");
        REQUIRE(tokens.at(i).type == Token::Type::HscppRequireSource);
        REQUIRE(tokens.at(i++).value == "hscpp_require_source");
        REQUIRE(tokens.at(i).type == Token::Type::HscppRequireIncludeDir);
        REQUIRE(tokens.at(i++).value == "hscpp_require_include_dir");
        REQUIRE(tokens.at(i).type == Token::Type::HscppRequireLibrary);
        REQUIRE(tokens.at(i++).value == "hscpp_require_library");
        REQUIRE(tokens.at(i).type == Token::Type::HscppRequireLibraryDir);
        REQUIRE(tokens.at(i++).value == "hscpp_require_library_dir");
        REQUIRE(tokens.at(i).type == Token::Type::HscppRequirePreprocessorDef);
        REQUIRE(tokens.at(i++).value == "hscpp_require_preprocessor_def");
        REQUIRE(tokens.at(i).type == Token::Type::HscppModule);
        REQUIRE(tokens.at(i++).value == "hscpp_module");
        REQUIRE(tokens.at(i).type == Token::Type::HscppIf);
        REQUIRE(tokens.at(i++).value == "hscpp_if");
        REQUIRE(tokens.at(i).type == Token::Type::HscppElif);
        REQUIRE(tokens.at(i++).value == "hscpp_elif");
        REQUIRE(tokens.at(i).type == Token::Type::HscppElse);
        REQUIRE(tokens.at(i++).value == "hscpp_else");
        REQUIRE(tokens.at(i).type == Token::Type::HscppEnd);
        REQUIRE(tokens.at(i++).value == "hscpp_end");
        REQUIRE(tokens.at(i).type == Token::Type::HscppReturn);
        REQUIRE(tokens.at(i++).value == "hscpp_return");
        REQUIRE(tokens.at(i).type == Token::Type::HscppTrack);
        REQUIRE(tokens.at(i++).value == "HSCPP_TRACK");
        REQUIRE(tokens.at(i).type == Token::Type::Bool);
        REQUIRE(tokens.at(i++).value == "true");
        REQUIRE(tokens.at(i).type == Token::Type::Bool);
        REQUIRE(tokens.at(i++).value == "false");
        REQUIRE(tokens.at(i).type == Token::Type::Unknown);
        REQUIRE(tokens.at(i++).value == "=");
        REQUIRE(tokens.at(i).type == Token::Type::Unknown);
        REQUIRE(tokens.at(i++).value == "&");
        REQUIRE(tokens.at(i).type == Token::Type::Unknown);
        REQUIRE(tokens.at(i++).value == "|");
        REQUIRE(tokens.at(i).type == Token::Type::Unknown);
        REQUIRE(tokens.at(i++).value == "^");
    }

    TEST_CASE("Lexer can lex various identifiers.")
    {
        std::string str = "a _b c_ MY_IDENTIFIER_448 Id3nt1fiEr";

        std::vector<Token> tokens;

        Lexer lexer;
        REQUIRE(lexer.Lex(str, tokens) );

        REQUIRE(tokens.at(0).type == Token::Type::Identifier);
        REQUIRE(tokens.at(0).value == "a");

        REQUIRE(tokens.at(1).type == Token::Type::Identifier);
        REQUIRE(tokens.at(1).value == "_b");

        REQUIRE(tokens.at(2).type == Token::Type::Identifier);
        REQUIRE(tokens.at(2).value == "c_");

        REQUIRE(tokens.at(3).type == Token::Type::Identifier);
        REQUIRE(tokens.at(3).value == "MY_IDENTIFIER_448");

        REQUIRE(tokens.at(4).type == Token::Type::Identifier);
        REQUIRE(tokens.at(4).value == "Id3nt1fiEr");
    }

    TEST_CASE("Lexer can lex various numbers.")
    {
        std::string str = "0 22. 0.22 932857.3872 867";

        std::vector<Token> tokens;

        Lexer lexer;
        REQUIRE(lexer.Lex(str, tokens));

        REQUIRE(tokens.at(0).type == Token::Type::Number);
        REQUIRE(tokens.at(0).value == "0");

        REQUIRE(tokens.at(1).type == Token::Type::Number);
        REQUIRE(tokens.at(1).value == "22.");

        REQUIRE(tokens.at(2).type == Token::Type::Number);
        REQUIRE(tokens.at(2).value == "0.22");

        REQUIRE(tokens.at(3).type == Token::Type::Number);
        REQUIRE(tokens.at(3).value == "932857.3872");

        REQUIRE(tokens.at(4).type == Token::Type::Number);
        REQUIRE(tokens.at(4).value == "867");
    }

    TEST_CASE("Lexer can lex various strings.")
    {
        std::string str = R"(
            "Hello, World!" "" "F" "@!(*$&"  "\\"
        )";

        std::vector<Token> tokens;

        Lexer lexer;
        REQUIRE(lexer.Lex(str, tokens));

        REQUIRE(tokens.at(0).type == Token::Type::String);
        REQUIRE(tokens.at(0).value == "Hello, World!");

        REQUIRE(tokens.at(1).type == Token::Type::String);
        REQUIRE(tokens.at(1).value == "");

        REQUIRE(tokens.at(2).type == Token::Type::String);
        REQUIRE(tokens.at(2).value == "F");

        REQUIRE(tokens.at(3).type == Token::Type::String);
        REQUIRE(tokens.at(3).value == "@!(*$&");

        REQUIRE(tokens.at(4).type == Token::Type::String);
        REQUIRE(tokens.at(4).value == "\\");
    }

    TEST_CASE("Lexer can lex various includes.")
    {
        std::string str = R"(
            #include<unordered_map>
            #include <> // Should lex correctly, even if it wouldn't compile...
            #  include                     <path/To/TheFile.cpp>
            #include "some/File.h"
            #  include"FILENAME"
        )";

        std::vector<Token> tokens;

        Lexer lexer;
        REQUIRE(lexer.Lex(str, tokens));

        REQUIRE(tokens.at(0).type == Token::Type::Include);
        REQUIRE(tokens.at(1).type == Token::Type::String);
        REQUIRE(tokens.at(1).value == "unordered_map");

        REQUIRE(tokens.at(2).type == Token::Type::Include);
        REQUIRE(tokens.at(3).type == Token::Type::String);
        REQUIRE(tokens.at(3).value == "");

        REQUIRE(tokens.at(4).type == Token::Type::Include);
        REQUIRE(tokens.at(5).type == Token::Type::String);
        REQUIRE(tokens.at(5).value == "path/To/TheFile.cpp");

        REQUIRE(tokens.at(6).type == Token::Type::Include);
        REQUIRE(tokens.at(7).type == Token::Type::String);
        REQUIRE(tokens.at(7).value == "some/File.h");

        REQUIRE(tokens.at(8).type == Token::Type::Include);
        REQUIRE(tokens.at(9).type == Token::Type::String);
        REQUIRE(tokens.at(9).value == "FILENAME");
    }

    TEST_CASE("Lexer can make it through complex C++ files.")
    {
        std::vector<Token> tokens;

        std::string clangLexerStr = CALL(FileToString, CLANG_FILES_PATH / "Lexer.cpp");
        std::string boostCrcStr = CALL(FileToString, BOOST_FILE_PATH / "crc.hpp");
        std::string boostFutureStr = CALL(FileToString, BOOST_FILE_PATH / "future.hpp");

        Lexer lexer;
        REQUIRE(lexer.Lex(clangLexerStr, tokens));
        REQUIRE(tokens.size() > 1000);

        tokens.clear();
        REQUIRE(lexer.Lex(boostCrcStr, tokens));
        REQUIRE(tokens.size() > 1000);

        tokens.clear();
        REQUIRE(lexer.Lex(boostFutureStr, tokens));
        REQUIRE(tokens.size() > 1000);
    }

    TEST_CASE("Lexer handles errors correctly.")
    {
        std::vector<Token> tokens;

        CALL(ValidateError, "\n\n\nhscpp_include(\"unterminated string);",
            LangError::Code::Lexer_UnterminatedString, 4, { "\"", "4", "unterminat" });

        // Adding a raw newline.
        CALL(ValidateError, "#include \"Good.h\"\n#include <bad\nhscpp_module(\"module\")",
            LangError::Code::Lexer_UnterminatedString, 3, { ">", "2", "bad\nhscpp_" });

        // Currently, only \" and \\ escape sequences are handled.
        CALL(ValidateError, R"("\n\n\"\\)",
            LangError::Code::Lexer_UnterminatedString, 1, { "\"", "1", R"(\n\n"\)" });

        CALL(ValidateError, "\"",
            LangError::Code::Lexer_UnterminatedString, 1, { "\"", "1", "" });
    }

}}