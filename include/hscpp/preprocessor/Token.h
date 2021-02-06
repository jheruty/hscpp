#pragma once

#include <string>

#include "hscpp/preprocessor/LangError.h"

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
            Inequivalent,      // !=
            LessThan,           // <
            LessThanOrEqual,    // <=
            GreaterThan,        // >
            GreaterThanOrEqual, // >=
            LogicalAnd,         // &&
            LogicalOr,          // ||
            Plus,               // +
            Minus,              // -
            Slash,              // /
            Star,               // *
            Exclamation,        // !

            Identifier,
            String,
            Number,
            Bool,

            Include,

            HscppRequireSource,
            HscppRequireIncludeDir,
            HscppRequireLibrary,
            HscppRequireLibraryDir,
            HscppRequirePreprocessorDef,
            HscppModule,
            HscppMessage,
            HscppIf,
            HscppElif,
            HscppElse,
            HscppEnd,
            HscppReturn,

            HscppTrack,
        };

        Type type = Type::Unknown;
        size_t line = LangError::NO_VALUE;
        size_t column = LangError::NO_VALUE;
        std::string value;
    };

}