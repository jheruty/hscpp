#pragma once

#include <string>

#include "hscpp/preprocessor/Token.h"
#include "hscpp/preprocessor/LangError.h"

namespace hscpp
{

    class Variant
    {
    public:
        enum class Type
        {
            Unknown,
            String,
            Number,
            Bool,
        };

        Variant() = default;
        explicit Variant(const std::string& val);
        explicit Variant(const char* pVal);
        explicit Variant(double val);
        explicit Variant(bool val);

        Type GetType() const;
        std::string GetTypeName() const;

        bool IsString() const;
        bool IsNumber() const;
        bool IsBool() const;

        bool IsTruthy() const;

        std::string StringVal() const;
        double NumberVal() const;
        bool BoolVal() const;

        std::string ToString() const;

    private:
        Type m_Type = Type::Unknown;

        std::string m_StringVal;
        double m_NumberVal = 0;
        bool m_BoolVal = false;
    };

    bool UnaryOp(const Token& op, const Variant& rhs, Variant& result, LangError& error);
    bool BinaryOp(const Token& op, const Variant& lhs, const Variant& rhs, Variant& result, LangError& error);

}