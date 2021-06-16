#pragma once

#include <vector>
#include <string>
#include <limits>

namespace hscpp
{

    class LangError
    {
    public:
        static constexpr size_t NO_VALUE = (std::numeric_limits<size_t>::max)();

        enum class Code
        {
            Success,                                                                //
                                                                                    //
            Lexer_UnterminatedString,                                               // (expected, start_line, beginning_of_string)
                                                                                    //
            Parser_FailedToParsePrefixExpression,                                   // (op)
            Parser_FailedToParseInfixExpression,                                    // (op)
            Parser_FailedToParseNumber,                                             // (number_val)
            Parser_NumberIsOutOfRange,                                              // (number_val)
            Parser_GroupExpressionMissingClosingParen,                              // ()
            Parser_IncludeMissingPath,                                              // ()
            Parser_HscppIfStmtMissingHscppEnd,                                      // (if_stmt_type)
            Parser_HscppStmtMissingOpeningParen,                                    // (stmt_type)
            Parser_HscppStmtMissingClosingParen,                                    // (stmt_type)
            Parser_HscppStmtArgumentMustBeStringLiteral,                            // (stmt_type)
            Parser_HscppStmtMissingCommaInArgumentList,                             // (stmt_type)
            Parser_HscppStmtExpectedStringLiteralInArgumentList,                    // (stmt_type)
            Parser_HscppStmtExpectedStringLiteralOrIdentifierInArgumentList,        // (stmt_type)
            Parser_HscppTrackMissingIdentifier,                                     //
            Parser_HscppTrackMissingString,                                         //
                                                                                    //
            Interpreter_UnableToResolveName,                                        // (name)
                                                                                    //
            Variant_OperandMustBeNumber,                                            // (op)
            Variant_OperandsDifferInType,                                           // (op, left, right)
                                                                                    //
            InternalError,                                                          // ()
        };

        LangError(Code errorCode);
        LangError(Code errorCode, size_t line, size_t column, const std::vector<std::string>& args);
        LangError(Code errorCode, size_t line, const std::vector<std::string>& args);
        LangError(Code errorCode, const std::vector<std::string>& args);

        Code ErrorCode() const;
        size_t Line() const;

        std::string ToString() const;
        size_t NumArgs() const;
        std::string GetArg(size_t i) const;

    private:
        Code m_ErrorCode;
        size_t m_Line = NO_VALUE;
        size_t m_Column = NO_VALUE;

        std::vector<std::string> m_Args;
    };

    class LangErrorCodeHasher
    {
    public:
        size_t operator()(LangError::Code errorCode) const;
    };

}