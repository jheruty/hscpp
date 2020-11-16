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

}}