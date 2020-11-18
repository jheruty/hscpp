#include <cassert>
#include <sstream>
#include <limits>
#include <iomanip>

#include "hscpp/preprocessor/Variant.h"

namespace hscpp
{

#define RETURN_WHEN_ERROR(bResult) if (!bResult) { return false; }

    Variant::Variant(const std::string& val)
    {
        m_Type = Type::String;
        m_StringVal = val;
    }

    Variant::Variant(const char* pVal)
        : Variant(std::string(pVal)) // Work around implicit const char* -> bool conversion.
    {}

    Variant::Variant(double val)
    {
        m_Type = Type::Number;
        m_NumberVal = val;
    }

    Variant::Variant(bool val)
    {
        m_Type = Type::Bool;
        m_BoolVal = val;
    }

    Variant::Type Variant::GetType() const
    {
        return m_Type;
    }

    std::string Variant::GetTypeName() const
    {
        switch (m_Type)
        {
            case Type::Unknown:
                return "Unknown";
            case Type::String:
                return "String";
            case Type::Number:
                return "Number";
            case Type::Bool:
                return "Bool";
            default:
                assert(false);
                break;
        }

        return "Unknown";
    }

    bool Variant::IsString() const
    {
        return m_Type == Type::String;
    }

    bool Variant::IsNumber() const
    {
        return m_Type == Type::Number;
    }

    bool Variant::IsBool() const
    {
        return m_Type == Type::Bool;
    }

    bool Variant::IsTruthy() const
    {
        switch (m_Type)
        {
            case Type::Unknown:
                return false;
            case Type::String:
                return !m_StringVal.empty();
            case Type::Number:
                return m_NumberVal != 0;
            case Type::Bool:
                return m_BoolVal;
            default:
                assert(false);
                break;
        }

        return false;
    }

    std::string Variant::StringVal() const
    {
        return m_StringVal;
    }

    double Variant::NumberVal() const
    {
        return m_NumberVal;
    }

    bool Variant::BoolVal() const
    {
        return m_BoolVal;
    }

    std::string Variant::ToString() const
    {
        switch (m_Type)
        {
            case Type::Unknown:
                return "<unknown>";
            case Type::String:
                return m_StringVal;
            case Type::Number:
            {
                // Get value of double as string with max precision.
                std::stringstream ss;
                ss << std::setprecision(std::numeric_limits<double>::max_digits10);
                ss << m_NumberVal;

                std::string maxPrecisionStr = ss.str();
                std::string cleanStr = maxPrecisionStr;

                // Remove extra trailing zeroes (ex. 1.1000 -> 1.1, 1.000 -> 1.).
                if (maxPrecisionStr.find(".") != std::string::npos)
                {
                    for (auto it = maxPrecisionStr.rbegin(); it != maxPrecisionStr.rend(); ++it )
                    {
                        if (*it != '0')
                        {
                            break;
                        }

                        cleanStr.pop_back();
                    }
                }

                // Remove decimal in numbers with no fractional value (ex. 1. -> 1).
                if (cleanStr.back() == '.')
                {
                    cleanStr.pop_back();
                }

                return cleanStr;
            }
            case Type::Bool:
                return m_BoolVal ? "true" : "false";
            default:
                assert(false);
                break;
        }

        return "<unknown>";
    }

    static bool ExpectNumeric(const Token& op, const Variant& variant, LangError& error)
    {
        if (!variant.IsNumber())
        {
            error = LangError(LangError::Code::Variant_OperandMustBeNumber, op.line, { op.value });
            return false;
        }

        return true;
    }

    static bool EqualityComparison(const Token& op,
        const Variant& lhs, const Variant& rhs, Variant& result, LangError& error)
    {
        if (lhs.GetType() != rhs.GetType())
        {
            error = LangError(LangError::Code::Variant_OperandsDifferInType,
                    op.line, { op.value, lhs.GetTypeName(), rhs.GetTypeName() });
            return false;
        }

        std::string lhsString = lhs.StringVal();
        double lhsNumber = lhs.NumberVal();
        bool lhsBool = lhs.BoolVal();

        std::string rhsString = rhs.StringVal();
        double rhsNumber = rhs.NumberVal();
        bool rhsBool = rhs.BoolVal();

        if (lhs.IsString())
        {
            switch (op.type)
            {
                case Token::Type::Equivalent:
                    result = Variant(lhsString == rhsString);
                    break;
                case Token::Type::Inequivalent:
                    result = Variant(lhsString != rhsString);
                    break;
                default:
                    error = LangError(LangError::Code::InternalError);
                    assert(false);
                    return false;
            }

            return true;
        }
        else if (lhs.IsNumber())
        {
            switch (op.type)
            {
                case Token::Type::Equivalent:
                    result = Variant(lhsNumber == rhsNumber);
                    break;
                case Token::Type::Inequivalent:
                    result = Variant(lhsNumber != rhsNumber);
                    break;
                default:
                    error = LangError(LangError::Code::InternalError);
                    assert(false);
                    return false;
            }

            return true;
        }
        else if (lhs.IsBool())
        {
            switch (op.type)
            {
                case Token::Type::Equivalent:
                    result = Variant(lhsBool == rhsBool);
                    break;
                case Token::Type::Inequivalent:
                    result = Variant(lhsBool != rhsBool);
                    break;
                default:
                    error = LangError(LangError::Code::InternalError);
                    assert(false);
                    return false;
            }

            return true;
        }
        else
        {
            error = LangError(LangError::Code::InternalError);
            assert(false);
            return false;
        }

        return true;
    }

    static bool LessOrGreaterComparison(const Token& op,
        const Variant& lhs, const Variant& rhs, Variant& result, LangError& error)
    {
        bool bResult = ExpectNumeric(op, lhs, error);
        RETURN_WHEN_ERROR(bResult);

        bResult = ExpectNumeric(op, rhs, error);
        RETURN_WHEN_ERROR(bResult);

        switch (op.type)
        {
            case Token::Type::LessThan:
                result = Variant(lhs.NumberVal() < rhs.NumberVal());
                break;
            case Token::Type::LessThanOrEqual:
                result = Variant(lhs.NumberVal() <= rhs.NumberVal());
                break;
            case Token::Type::GreaterThan:
                result = Variant(lhs.NumberVal() > rhs.NumberVal());
                break;
            case Token::Type::GreaterThanOrEqual:
                result = Variant(lhs.NumberVal() >= rhs.NumberVal());
                break;
            default:
                error = LangError(LangError::Code::InternalError);
                assert(false);
                return false;
        }

        return true;
    }

    static bool LogicalComparison(const Token& op,
        const Variant& lhs, const Variant& rhs, Variant& result, LangError& error)
    {
        switch (op.type)
        {
            case Token::Type::LogicalAnd:
                result = Variant(lhs.IsTruthy() && rhs.IsTruthy());
                break;
            case Token::Type::LogicalOr:
                result = Variant(lhs.IsTruthy() || rhs.IsTruthy());
                break;
            default:
                error = LangError(LangError::Code::InternalError);
                assert(false);
                return false;
        }

        return true;
    }

    static bool ArithmeticOp(const Token& op,
        const Variant& lhs, const Variant& rhs, Variant& result, LangError& error)
    {
        bool bResult = ExpectNumeric(op, lhs, error);
        RETURN_WHEN_ERROR(bResult);

        bResult = ExpectNumeric(op, rhs, error);
        RETURN_WHEN_ERROR(bResult);

        switch (op.type)
        {
            case Token::Type::Plus:
                result = Variant(lhs.NumberVal() + rhs.NumberVal());
                break;
            case Token::Type::Minus:
                result = Variant(lhs.NumberVal() - rhs.NumberVal());
                break;
            case Token::Type::Slash:
                result = Variant(lhs.NumberVal() / rhs.NumberVal());
                break;
            case Token::Type::Star:
                result = Variant(lhs.NumberVal() * rhs.NumberVal());
                break;
            default:
                assert(false);
                return false;
        }

        return true;
    }

    static bool Negate(const Token& op, const Variant& rhs, Variant& result, LangError& error)
    {
        bool bResult = ExpectNumeric(op, rhs, error);
        RETURN_WHEN_ERROR(bResult);

        result = Variant(-rhs.NumberVal());

        return true;
    }

    static bool Not(const Variant& rhs, Variant& result)
    {
        result = Variant(!rhs.IsTruthy());
        return true;
    }

    bool UnaryOp(const Token& op, const Variant& rhs, Variant& result, LangError& error)
    {
        switch (op.type)
        {
            case Token::Type::Minus:
                return Negate(op, rhs, result, error);
            case Token::Type::Exclamation:
                return Not(rhs, result);
            default:
                assert(false);
                return false;
        }

        return false;
    }

    bool BinaryOp(const Token& op,
        const Variant& lhs, const Variant& rhs, Variant& result, LangError& error)
    {
        switch (op.type)
        {
            case Token::Type::Equivalent:
            case Token::Type::Inequivalent:
                return EqualityComparison(op, lhs, rhs, result, error);
            case Token::Type::LessThan:
            case Token::Type::LessThanOrEqual:
            case Token::Type::GreaterThan:
            case Token::Type::GreaterThanOrEqual:
                return LessOrGreaterComparison(op, lhs, rhs, result, error);
            case Token::Type::LogicalAnd:
            case Token::Type::LogicalOr:
                return LogicalComparison(op, lhs, rhs, result, error);
            case Token::Type::Plus:
            case Token::Type::Minus:
            case Token::Type::Slash:
            case Token::Type::Star:
                return ArithmeticOp(op, lhs, rhs, result, error);
            default:
                assert(false);
                return false;
        }

        return false;
    }

}