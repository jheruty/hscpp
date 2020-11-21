#include "catch/catch.hpp"
#include "common/Common.h"
#include "hscpp/Platform.h"
#include "hscpp/Util.h"
#include "hscpp/preprocessor/Lexer.h"
#include "hscpp/preprocessor/Parser.h"
#include "hscpp/preprocessor/Interpreter.h"

namespace hscpp { namespace test
{

    static Interpreter::Result RunInterpreter(const std::string& program, const VarStore& varStore)
    {
        Lexer lexer;
        Parser parser;
        Interpreter interpreter;

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

        Interpreter::Result result;
        bResult = interpreter.Evaluate(*pRootStmt, varStore, result);
        if (!bResult)
        {
            FAIL(interpreter.GetLastError().ToString());
        }

        return result;
    }

    static void ValidateSingleMessage(const Interpreter::Result& result, const std::string& message)
    {
        REQUIRE(result.hscppRequires.empty());
        REQUIRE(result.hscppModules.empty());
        REQUIRE(result.includePaths.empty());

        REQUIRE(result.hscppMessages.size() == 1);
        REQUIRE(result.hscppMessages.at(0) == message);
    }

    static void ValidateExpression(const std::string& expression, const std::string& expected, const VarStore& store)
    {
        std::string program = "hscpp_if((" + expression + ") == " + expected + ")\n"
                + "hscpp_message(\"PASS\")\n"
                + "hscpp_else()\n"
                + "hscpp_message(\"FAIL\")\n"
                + "hscpp_end()";

        Interpreter::Result result = CALL(RunInterpreter, program, store);
        REQUIRE(!result.hscppMessages.empty());

        INFO("(" + expression + ") == (" + expected + ")")
        REQUIRE(result.hscppMessages.at(0) == "PASS");
    }

    static void ValidateError(const std::string& program,
        const VarStore& store,
        const LangError::Code& expectedCode,
        size_t expectedLine,
        const std::vector<std::string>& expectedArgs)
    {
        Lexer lexer;
        Parser parser;
        Interpreter interpreter;

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

        Interpreter::Result result;
        REQUIRE_FALSE(interpreter.Evaluate(*pRootStmt, store, result));
        CALL(ValidateError, interpreter.GetLastError(), expectedCode, expectedLine, expectedArgs);
    }

    TEST_CASE("Interpreter can evaluate a simple if chain.")
    {
        std::string program = R"(
            hscpp_if (var1);
                hscpp_message("if");
            hscpp_elif (var2);
                hscpp_message("elif");
            hscpp_else ();
                hscpp_message("else");
            hscpp_end ();
        )";

        VarStore store;
        store.SetVar("var1", Variant(true));
        store.SetVar("var2", Variant(true));

        Interpreter::Result result = CALL(RunInterpreter, program, store);
        CALL(ValidateSingleMessage, result, "if");

        store.SetVar("var1", Variant(false));

        result = CALL(RunInterpreter, program, store);
        CALL(ValidateSingleMessage, result, "elif");

        store.SetVar("var2", Variant(false));

        result = CALL(RunInterpreter, program, store);
        CALL(ValidateSingleMessage, result, "else");
    }

    TEST_CASE("Interpreter can evaluate basic expressions.")
    {
        VarStore store;

        store.SetVar("a", Variant(1.0));
        CALL(ValidateExpression, "a", "1", store);
        CALL(ValidateExpression, "-a", "-1", store);
        CALL(ValidateExpression, "--a", "1", store); // negative negative a, decrement operator not supported
        CALL(ValidateExpression, "!a", "false", store);
        CALL(ValidateExpression, "!!a", "true", store);

        store.SetVar("a", Variant(1.5));
        CALL(ValidateExpression, "a", "1.5", store);

        store.SetVar("a", Variant(2.0));
        store.SetVar("b", Variant(4.0));
        store.SetVar("c", Variant(-22.0));
        CALL(ValidateExpression, "a + b", "6", store);
        CALL(ValidateExpression, "a + b", "6", store);
        CALL(ValidateExpression, "-a - b * c", "86", store);
        CALL(ValidateExpression, "-(a - b * c)", "-90", store);
        CALL(ValidateExpression, "-((a - b) * c))", "-44", store);
        CALL(ValidateExpression, "1 / 2", "0.5", store);

        CALL(ValidateExpression, "1 == 1", "true", store);
        CALL(ValidateExpression, "1 == 2", "false", store);
        CALL(ValidateExpression, "1 != 2", "true", store);
        CALL(ValidateExpression, "1 != 1", "false", store);
        CALL(ValidateExpression, R"( "x" == "x" )", "true", store);
        CALL(ValidateExpression, R"( "x" == "y" )", "false", store);
        CALL(ValidateExpression, R"( "x" != "y" )", "true", store);
        CALL(ValidateExpression, R"( "x" == "y" )", "false", store);
        CALL(ValidateExpression, R"( true == true )", "true", store);
        CALL(ValidateExpression, R"( true == false )", "false", store);

        store.SetVar("a", Variant(100.0));
        store.SetVar("b", Variant(200.0));
        store.SetVar("c", Variant(100.0));
        CALL(ValidateExpression, "a < b", "true", store);
        CALL(ValidateExpression, "a > b", "false", store);
        CALL(ValidateExpression, "a <= b", "true", store);
        CALL(ValidateExpression, "a >= b", "false", store);
        CALL(ValidateExpression, "a < c", "false", store);
        CALL(ValidateExpression, "a <= c", "true", store);
        CALL(ValidateExpression, "c < a", "false", store);
        CALL(ValidateExpression, "c <= a", "true", store);
        CALL(ValidateExpression, "c <= a && a < b", "true", store);
        CALL(ValidateExpression, "c < a && a < b", "false", store);
        CALL(ValidateExpression, "c < a || a < b", "true", store);
        CALL(ValidateExpression, "!(c < a || a < b)", "false", store);
    }

    TEST_CASE("Interpreter handles errors correctly.")
    {
        VarStore store;
        CALL(ValidateError, "\n\n\nhscpp_if(unknown_identifier > 2) hscpp_end()", store,
            LangError::Code::Interpreter_UnableToResolveName, 4, { "unknown_identifier" });

        store.SetVar("a", Variant(false));
        store.SetVar("b", Variant("An ordinary string."));
        CALL(ValidateError, "\nhscpp_if(a == b) hscpp_end()", store,
            LangError::Code::Variant_OperandsDifferInType, 2, { "==", "Bool", "String" });
        CALL(ValidateError, "\nhscpp_if(a != b) hscpp_end()", store,
            LangError::Code::Variant_OperandsDifferInType, 2, { "!=", "Bool", "String" });
        CALL(ValidateError, "\nhscpp_if(a < b) hscpp_end()", store,
            LangError::Code::Variant_OperandMustBeNumber, 2, { "<" });
        CALL(ValidateError, "\nhscpp_if(a > b) hscpp_end()", store,
            LangError::Code::Variant_OperandMustBeNumber, 2, { ">" });
        CALL(ValidateError, "\nhscpp_if(a <= b) hscpp_end()", store,
            LangError::Code::Variant_OperandMustBeNumber, 2, { "<=" });
        CALL(ValidateError, "\nhscpp_if(a >= b) hscpp_end()", store,
            LangError::Code::Variant_OperandMustBeNumber, 2, { ">=" });
        CALL(ValidateError, "\nhscpp_if(a + b) hscpp_end()", store,
            LangError::Code::Variant_OperandMustBeNumber, 2, { "+" });
        CALL(ValidateError, "\nhscpp_if(a - b) hscpp_end()", store,
            LangError::Code::Variant_OperandMustBeNumber, 2, { "-" });
        CALL(ValidateError, "\nhscpp_if(a * b) hscpp_end()", store,
            LangError::Code::Variant_OperandMustBeNumber, 2, { "*" });
        CALL(ValidateError, "\nhscpp_if(a / b) hscpp_end()", store,
            LangError::Code::Variant_OperandMustBeNumber, 2, { "/" });
    }

}}