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

        void Visit(const IncludeStmt& includeStmt) override
        {
            m_Stream << "\n(include \"" << includeStmt.path << "\")";
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
            m_Stream << nameExpr.value;
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

                hscpp_require_source("source.cpp", "AnotherSource.cpp")
                hscpp_require_include_dir("path/to/include", "another-Path/Include")
                hscpp_require_library("lib.so", "lib.a")
                hscpp_require_library_dir("path/to/lib_dir", "PATH/T-o/DIR/")
                hscpp_require_preprocessor_def("Hello", Goodbye);
                // hscpp_require_source("commented out")
                /* hscpp_require_source("commented out") */

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
                        (hscpp_require_source "source.cpp" "AnotherSource.cpp")
                        (hscpp_require_include_dir "path/to/include" "another-Path/Include")
                        (hscpp_require_library "lib.so" "lib.a")
                        (hscpp_require_library_dir "path/to/lib_dir" "PATH/T-o/DIR/")
                        (hscpp_require_preprocessor_def "Hello" "Goodbye")
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
    }

}}
