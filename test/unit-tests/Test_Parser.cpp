#include <sstream>
#include <iostream>
#include <cassert>

#include "catch/catch.hpp"
#include "common/Common.h"
#include "hscpp/Platform.h"
#include "hscpp/Util.h"
#include "hscpp/preprocessor/Lexer.h"
#include "hscpp/preprocessor/Parser.h"
#include "hscpp/preprocessor/Ast.h"

#include "hscpp/preprocessor/Interpreter.h"

namespace hscpp { namespace test
{

    class AstChecker : public IAstVisitor
    {
    public:
        std::string Str()
        {
            return m_Stream.str();
        }

        std::string Format(const std::string& str)
        {
            // Trim all lines to make it easier to compare two trees. Spacing within parentheses
            // still has to match, but, in practice, this is easy to do.
            std::stringstream ss;
            ss << str;

            std::string formatted;

            std::string line;
            while (std::getline(ss, line))
            {
                formatted += util::Trim(line) + "\n";
            }

            return formatted;
        }

        void Visit(const BlockStmt& blockStmt) override
        {
            m_Stream << "\n(block ";
            for (size_t i = 0; i < blockStmt.statements.size(); ++i)
            {
                blockStmt.statements.at(i)->Accept(*this);

                if (i < blockStmt.statements.size() - 1)
                {
                    m_Stream << " ";
                }
            }
            m_Stream << "\n)";
        }

        void Visit(const IncludeStmt& includeStmt) override
        {
            m_Stream << "\n(include \"" << includeStmt.path << "\")";
        }

        void Visit(const HscppIfStmt& ifStmt) override
        {
            if (ifStmt.conditions.size() > 0)
            {
                for (size_t i = 0; i < ifStmt.conditions.size(); ++i)
                {
                    std::string name = (i == 0) ? "hscpp_if" : "hscpp_elif";

                    m_Stream << "\n(" << name << " ";
                    ifStmt.conditions.at(i)->Accept(*this);

                    if (i < ifStmt.conditions.size() - 1)
                    {
                        m_Stream << " ";
                    }

                    ifStmt.conditionalBlocks.at(i)->Accept(*this);
                    m_Stream << "\n)";
                }
            }

            if (ifStmt.pElseBlock != nullptr)
            {
                m_Stream << "\n(hscpp_else" << " ";
                ifStmt.pElseBlock->Accept(*this);
                m_Stream << "\n)";
            }
        }

        void Visit(const HscppReturnStmt& returnStmt) override
        {
            HSCPP_UNUSED_PARAM(returnStmt);
            m_Stream << "\n(hscpp_return)";
        }

        void Visit(const HscppRequireStmt& requireStmt) override
        {
            m_Stream << "\n(";
            switch (requireStmt.token.type)
            {
                case Token::Type::HscppRequireSource:
                    m_Stream << "hscpp_require_source";
                    break;
                case Token::Type::HscppRequireIncludeDir:
                    m_Stream << "hscpp_require_include_dir";
                    break;
                case Token::Type::HscppRequireLibrary:
                    m_Stream << "hscpp_require_library";
                    break;
                case Token::Type::HscppRequireLibraryDir:
                    m_Stream << "hscpp_require_library_dir";
                    break;
                case Token::Type::HscppRequirePreprocessorDef:
                    m_Stream << "hscpp_require_preprocessor_def";
                    break;
                default:
                    REQUIRE(false);
                    break;
            }
            m_Stream << " ";

            for (size_t i = 0; i < requireStmt.parameters.size(); ++i)
            {
                m_Stream << "\"" << requireStmt.parameters.at(i) << "\"";

                if (i < requireStmt.parameters.size() - 1)
                {
                    m_Stream << " ";
                }
            }

            m_Stream << ")";
        }

        void Visit(const HscppModuleStmt& moduleStmt) override
        {
            m_Stream << "\n(hscpp_module \"" << moduleStmt.module << "\")";
        }

        void Visit(const HscppMessageStmt& messageStmt) override
        {
            m_Stream << "\n(hscpp_message \"" << messageStmt.message << "\")";
        }

        void Visit(const UnaryExpr& unaryExpr) override
        {
            m_Stream << "(" << unaryExpr.op.value << " ";
            unaryExpr.pRightExpr->Accept(*this);
            m_Stream << ")";
        }

        void Visit(const BinaryExpr& binaryExpr) override
        {
            m_Stream << "(" << binaryExpr.op.value << " ";
            binaryExpr.pLeftExpr->Accept(*this);
            m_Stream << " ";
            binaryExpr.pRightExpr->Accept(*this);
            m_Stream << ")";
        }

        void Visit(const NameExpr& nameExpr) override
        {
            m_Stream << nameExpr.name.value;
        }

        void Visit(const StringLiteralExpr& stringLiteralExpr) override
        {
            m_Stream << stringLiteralExpr.value;
        }

        void Visit(const NumberLiteralExpr& numberLiteralExpr) override
        {
            m_Stream << numberLiteralExpr.value;
        }

        void Visit(const BoolLiteralExpr& boolLiteralExpr) override
        {
            m_Stream << std::boolalpha << boolLiteralExpr.value << std::noboolalpha;
        }

    private:
        std::stringstream m_Stream;
    };

    static void ValidateAst(const std::string& program, const std::string& expected)
    {
        Lexer lexer;
        Parser parser;

        std::vector<Token> tokens;
        bool bResult = lexer.Lex(program, tokens);
        if (!bResult)
        {
            FAIL(lexer.GetLastError().ToString());
        }

        std::unique_ptr<Stmt> pRootStmt;
        bResult = parser.Parse(tokens, pRootStmt);
        if (!bResult)
        {
            FAIL(parser.GetLastError().ToString());
        }

        AstChecker astChecker;
        pRootStmt->Accept(astChecker);

        REQUIRE(astChecker.Format(astChecker.Str()) == astChecker.Format(expected));
    }

    static void ValidateExpression(const std::string& expression, const std::string& expected)
    {
        std::string program = "hscpp_if(" + expression + ") hscpp_end()";
        std::string fullExpected = "\n(block\n(hscpp_if " + expected + "\n(block\n)\n)\n)";

        CALL(ValidateAst, program, fullExpected);
    }

    static void ValidateError(const std::string& program,
        const LangError::Code& expectedCode,
        size_t expectedLine,
        const std::vector<std::string>& expectedArgs)
    {
        Lexer lexer;
        Parser parser;

        std::vector<Token> tokens;
        bool bResult = lexer.Lex(program, tokens);
        if (!bResult)
        {
            FAIL(lexer.GetLastError().ToString());
        }

        std::unique_ptr<Stmt> pRootStmt;
        REQUIRE_FALSE(parser.Parse(tokens, pRootStmt));
        CALL(ValidateError, parser.GetLastError(), expectedCode, expectedLine, expectedArgs);
    }

    TEST_CASE("AstChecker can format correctly.")
    {
        // Sanity check, formatter is very simple and simply trims all lines for easier comparison.
        std::string formatted = R"(
            (first (second))
            (third)
            (fourth(fifth)       )
        )";

        std::string compare = "\n(first (second))\n(third)\n(fourth(fifth)       )\n\n";

        AstChecker astChecker;
        REQUIRE(astChecker.Format(formatted) == compare);
        REQUIRE(astChecker.Format(formatted) == astChecker.Format(compare));
    }

    TEST_CASE("Parser can parse an AST with all keywords.")
    {
        std::string program = R"(
            hscpp_if (true)
                #include <unordered_map>
                #include "MyHeader.hpp"

                class RuntimeObject
                {
                    HSCPP_TRACK(RuntimeObject, "RuntimeObject");
                };

                hscpp_require_source("source.cpp", "AnotherSource.cpp")
                hscpp_require_include_dir("path/to/include", "another-Path/Include")
                hscpp_require_library("lib.so", "lib.a")
                hscpp_require_library_dir("path/to/lib_dir", "PATH/T-o/DIR/")
                hscpp_require_preprocessor_def("Hello", Goodbye);
                // hscpp_require_source("commented out")
                /* hscpp_require_source("commented out") */
                hscpp_return()

                IDENTIFIER_TO_IGNORE      "StringToIgnore"

                hscpp_if (false);
                    hscpp_module("MyModule")
                hscpp_elif (x + y < 12)
                    hscpp_message("Hello, World!")
                hscpp_elif (x - y <= 12)
                hscpp_elif (x / y > 12.)
                hscpp_elif (x * y >= 12.0)
                hscpp_elif (x == 12.0 && (x + 1 != 13.5))
                hscpp_elif (x == 12.0 || (x + 1 != 13.5))
                hscpp_else()
                hscpp_end();

            hscpp_end()
                    )";

        std::string expected = R"(
            (block
                (hscpp_if true
                    (block
                        (include "unordered_map")
                        (include "MyHeader.hpp")
                        (hscpp_module "@RuntimeObject")
                        (hscpp_require_source "source.cpp" "AnotherSource.cpp")
                        (hscpp_require_include_dir "path/to/include" "another-Path/Include")
                        (hscpp_require_library "lib.so" "lib.a")
                        (hscpp_require_library_dir "path/to/lib_dir" "PATH/T-o/DIR/")
                        (hscpp_require_preprocessor_def "Hello" "Goodbye")
                        (hscpp_return)
                        (hscpp_if false
                            (block
                                (hscpp_module "MyModule")
                            )
                        )
                        (hscpp_elif (< (+ x y) 12)
                            (block
                                (hscpp_message "Hello, World!")
                            )
                        )
                        (hscpp_elif (<= (- x y) 12)
                            (block
                            )
                        )
                        (hscpp_elif (> (/ x y) 12)
                            (block
                            )
                        )
                        (hscpp_elif (>= (* x y) 12)
                            (block
                            )
                        )
                        (hscpp_elif (&& (== x 12) (!= (+ x 1) 13.5))
                            (block
                            )
                        )
                        (hscpp_elif (|| (== x 12) (!= (+ x 1) 13.5))
                            (block
                            )
                        )
                        (hscpp_else
                            (block
                            )
                        )
                    )
                )
            ))";

        CALL(ValidateAst, program, expected);
    }

    TEST_CASE("Parser can parse precedence correctly.")
    {
        CALL(ValidateExpression, "a + b * c - d / e", "(- (+ a (* b c)) (/ d e))");
        CALL(ValidateExpression, "a * b / c", "(/ (* a b) c)");
        CALL(ValidateExpression, "a + b - c", "(- (+ a b) c)");
        CALL(ValidateExpression, "(a + b) * c", "(* (+ a b) c)");
        CALL(ValidateExpression, "(a - b) / c", "(/ (- a b) c)");
        CALL(ValidateExpression, "c < a && a < b", "(&& (< c a) (< a b))");
        CALL(ValidateExpression, "c < a && a < b == true", "(&& (< c a) (== (< a b) true))");
        CALL(ValidateExpression, "a || b && c || d && e", "(|| (|| a (&& b c)) (&& d e))");
        CALL(ValidateExpression, "a && b || c && d || e", "(|| (|| (&& a b) (&& c d)) e)");
        CALL(ValidateExpression, "a || b || c || d || e || f && g", "(|| (|| (|| (|| (|| a b) c) d) e) (&& f g))");
        CALL(ValidateExpression, "a > b < c", "(< (> a b) c)");
        CALL(ValidateExpression, "a >= b && c <= d || e + f * g == 22 && f != 2",
                "(|| (&& (>= a b) (<= c d)) (&& (== (+ e (* f g)) 22) (!= f 2)))");
        CALL(ValidateExpression, "---a", "(- (- (- a)))");
        CALL(ValidateExpression, "!!!a", "(! (! (! a)))");
        CALL(ValidateExpression, "-!!-a", "(- (! (! (- a))))");
    }

    TEST_CASE("Parser handles errors correctly.")
    {
        CALL(ValidateError, "\nhscpp_if(|a) hscpp_end()",
            LangError::Code::Parser_FailedToParsePrefixExpression, 2, { "|" });
        CALL(ValidateError, "\n\n\n\nhscpp_if((a + b) & c) hscpp_end()",
            LangError::Code::Parser_FailedToParseInfixExpression, 5, { "&" });
        // Skip Parser_FailedToParse number, since that should be caught in the Lexer.
        CALL(ValidateError, "\n\nhscpp_if(999999989999999998798798918237598123798123501239502391579821378951329715329898512439871532983210521398598125609865019826398062109839801236509812365981623590862130958612309868921356981230509812365089123598012365891230568962569851239865369823568169802069832098662019388902136598102359801235089123659802316598062139580623108937987912310591273598162359816235986213985691236598231659826315986213598612398569128365982136598623159862139506123598123650912380918230981235)\nhscpp_end()",
            LangError::Code::Parser_NumberIsOutOfRange, 3, { "999999989999999998798798918237598123798123501239502391579821378951329715329898512439871532983210521398598125609865019826398062109839801236509812365981623590862130958612309868921356981230509812365089123598012365891230568962569851239865369823568169802069832098662019388902136598102359801235089123659802316598062139580623108937987912310591273598162359816235986213985691236598231659826315986213598612398569128365982136598623159862139506123598123650912380918230981235" });
        CALL(ValidateError, "\n\n\n\n\nhscpp_if((a * (b * c + d) hscpp_end()",
            LangError::Code::Parser_GroupExpressionMissingClosingParen, 6, {});
        CALL(ValidateError, "\n#include ",
            LangError::Code::Parser_IncludeMissingPath, 2, {});
        CALL(ValidateError, "\n\n\n\n\n\nhscpp_if(true)\nhscpp_elif(true)\n\n\n",
            LangError::Code::Parser_HscppIfStmtMissingHscppEnd, 8, { "hscpp_elif" });
        CALL(ValidateError, "hscpp_if a + 1) hscpp_end())",
            LangError::Code::Parser_HscppStmtMissingOpeningParen, 1, { "hscpp_if" });
        CALL(ValidateError, "hscpp_if(q / 498.5)\n\nhscpp_elif false) hscpp_end())",
            LangError::Code::Parser_HscppStmtMissingOpeningParen, 3, { "hscpp_elif" });
        CALL(ValidateError, "\n\nhscpp_require_include_dir \"test\")",
            LangError::Code::Parser_HscppStmtMissingOpeningParen, 3, { "hscpp_require_include_dir" });
        CALL(ValidateError, "\n\nhscpp_module \"test\")",
            LangError::Code::Parser_HscppStmtMissingOpeningParen, 3, { "hscpp_module" });
        CALL(ValidateError, "\nhscpp_message \"test\")",
            LangError::Code::Parser_HscppStmtMissingOpeningParen, 2, { "hscpp_message" });
        CALL(ValidateError, "hscpp_if(a + 1 hscpp_end()",
            LangError::Code::Parser_HscppStmtMissingClosingParen, 1, { "hscpp_if" });
        CALL(ValidateError, "hscpp_if(q / 498.5)\n\nhscpp_elif(false hscpp_end())",
            LangError::Code::Parser_HscppStmtMissingClosingParen, 3, { "hscpp_elif" });
        CALL(ValidateError, "hscpp_if(true) hscpp_end)",
            LangError::Code::Parser_HscppStmtMissingOpeningParen, 1, { "hscpp_end" });
        CALL(ValidateError, "\nhscpp_if(true) hscpp_end(",
            LangError::Code::Parser_HscppStmtMissingClosingParen, 2, { "hscpp_end" });
        CALL(ValidateError, "\n\n\nhscpp_return",
            LangError::Code::Parser_HscppStmtMissingOpeningParen, 4, { "hscpp_return" });
        CALL(ValidateError, "\n\nhscpp_return(",
            LangError::Code::Parser_HscppStmtMissingClosingParen, 3, { "hscpp_return" });
        CALL(ValidateError, "\n\nhscpp_module(\"test\"",
            LangError::Code::Parser_HscppStmtMissingClosingParen, 3, { "hscpp_module" });
        CALL(ValidateError, "\nhscpp_message(\"test\"",
            LangError::Code::Parser_HscppStmtMissingClosingParen, 2, { "hscpp_message" });
        CALL(ValidateError, "\n\n\n\nhscpp_module(not * a + string)",
            LangError::Code::Parser_HscppStmtArgumentMustBeStringLiteral, 5, { "hscpp_module" });
        CALL(ValidateError, "\n\n\nhscpp_message(still_not_a_string)",
            LangError::Code::Parser_HscppStmtArgumentMustBeStringLiteral, 4, { "hscpp_message" });
        CALL(ValidateError, "\nhscpp_require_include_dir(\"good/path/to/file\", bad/path/to/file)",
            LangError::Code::Parser_HscppStmtExpectedStringLiteralInArgumentList, 2, { "hscpp_require_include_dir" });
        CALL(ValidateError, "\nhscpp_require_preprocessor_def(\"DEF1\", DEF2, 1.0)",
            LangError::Code::Parser_HscppStmtExpectedStringLiteralOrIdentifierInArgumentList, 2, { "hscpp_require_preprocessor_def" });
        CALL(ValidateError, "\n\nhscpp_require_source(\"source1\" \"source2\")",
            LangError::Code::Parser_HscppStmtMissingCommaInArgumentList, 3, { "hscpp_require_source" });
        CALL(ValidateError, "HSCPP_TRACK",
            LangError::Code::Parser_HscppStmtMissingOpeningParen, 1, { "HSCPP_TRACK" });
        CALL(ValidateError, "\n\nHSCPP_TRACK(\"Hello\"",
            LangError::Code::Parser_HscppTrackMissingIdentifier, 3, {});
        CALL(ValidateError, "\nHSCPP_TRACK(Identifier1, Identifier2)",
            LangError::Code::Parser_HscppTrackMissingString, 2, {});
        CALL(ValidateError, "\n\n\nHSCPP_TRACK(Intentifer, \"String\"",
            LangError::Code::Parser_HscppStmtMissingClosingParen, 4, { "HSCPP_TRACK" });
    }

}}
