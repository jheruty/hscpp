#pragma once

#include <string>

namespace hscpp
{

    struct Token
    {
        enum class Type
        {
            Unknown,

            LeftParen,          // (
            RightParen,         // )
            Comma,              // ,
            Equivalent,         // ==
            LessThan,           // <
            LessThanOrEqual,    // <=
            GreaterThan,        // >
            GreaterThanOrEqual, // >=
            LogicalAnd,         // &&
            LogicalOr,          // ||

            Identifier,
            Number,
            String,

            Include,

            HscppRequireSource,
            HscppRequireIncludeDir,
            HscppRequireLibrary,
            HscppRequireLibraryDir,
            HscppRequirePreprocessorDef,
            HscppModule,
            HscppIf,
            HscppElif,
            HscppElse,
            HscppEnd,
        };

        Type type = Type::Unknown;
        size_t line = 0;
        size_t column = 0;
        std::string value;
    };

}