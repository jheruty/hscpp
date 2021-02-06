#include <stdexcept>
#include <cassert>

#include "hscpp/preprocessor/Interpreter.h"
#include "hscpp/Platform.h"

namespace hscpp
{

    bool Interpreter::Evaluate(const Stmt& rootStmt, const VarStore& varStore, Result& result)
    {
        Reset(varStore, result);

        try
        {
            rootStmt.Accept(*this);
        }
        catch (ReturnFromInterpreter&)
        {
            // hscpp_return encountered, exit successfully.
            return true;
        }
        catch (std::runtime_error&)
        {
            return false;
        }

        return true;
    }

    LangError Interpreter::GetLastError()
    {
        return m_Error;
    }

    void Interpreter::Reset(const VarStore& varStore, Result& result)
    {
        m_pVarStore = &varStore;

        result = Result();
        m_pResult = &result;

        m_Error = LangError(LangError::Code::Success);
    }

    void Interpreter::Visit(const BlockStmt& blockStmt)
    {
        for (const auto& pStatement : blockStmt.statements)
        {
            pStatement->Accept(*this);
        }
    }

    void Interpreter::Visit(const IncludeStmt& includeStmt)
    {
        m_pResult->includePaths.push_back(includeStmt.path);
    }

    void Interpreter::Visit(const HscppIfStmt& ifStmt)
    {
        bool bMatchedCondition = false;

        for (size_t i = 0; i < ifStmt.conditions.size(); ++i)
        {
            ifStmt.conditions.at(i)->Accept(*this);
            if (PopResult().IsTruthy())
            {
                ifStmt.conditionalBlocks.at(i)->Accept(*this);
                bMatchedCondition = true;
                break;
            }
        }

        if (!bMatchedCondition && ifStmt.pElseBlock != nullptr)
        {
            ifStmt.pElseBlock->Accept(*this);
        }
    }

    void Interpreter::Visit(const HscppReturnStmt& returnStmt)
    {
        HSCPP_UNUSED_PARAM(returnStmt);
        throw ReturnFromInterpreter();
    }

    void Interpreter::Visit(const HscppRequireStmt& requireStmt)
    {
        HscppRequire hscppRequire;
        hscppRequire.name = requireStmt.token.value;
        hscppRequire.line = requireStmt.token.line;

        switch (requireStmt.token.type)
        {
            case Token::Type::HscppRequireSource:
                hscppRequire.type = HscppRequire::Type::Source;
                break;
            case Token::Type::HscppRequireIncludeDir:
                hscppRequire.type = HscppRequire::Type::IncludeDir;
                break;
            case Token::Type::HscppRequireLibrary:
                hscppRequire.type = HscppRequire::Type::Library;
                break;
            case Token::Type::HscppRequireLibraryDir:
                hscppRequire.type = HscppRequire::Type::LibraryDir;
                break;
            case Token::Type::HscppRequirePreprocessorDef:
                hscppRequire.type = HscppRequire::Type::PreprocessorDef;
                break;
            default:
                assert(false);
                break;
        }

        for (const auto& parameter : requireStmt.parameters)
        {
            hscppRequire.values.push_back(Interpolate(parameter));
        }

        m_pResult->hscppRequires.push_back(hscppRequire);
    }

    void Interpreter::Visit(const HscppModuleStmt& moduleStmt)
    {
        m_pResult->hscppModules.push_back(Interpolate(moduleStmt.module));
    }

    void Interpreter::Visit(const HscppMessageStmt& messageStmt)
    {
        m_pResult->hscppMessages.push_back(Interpolate(messageStmt.message));
    }

    void Interpreter::Visit(const UnaryExpr& unaryExpr)
    {
        unaryExpr.pRightExpr->Accept(*this);

        Variant result;
        LangError error(LangError::Code::Success);

        if (!UnaryOp(unaryExpr.op, PopResult(), result, error))
        {
            ThrowError(error);
        }

        m_VariantStack.push(result);
    }

    void Interpreter::Visit(const BinaryExpr& binaryExpr)
    {
        binaryExpr.pRightExpr->Accept(*this);
        binaryExpr.pLeftExpr->Accept(*this);

        Variant left = PopResult();
        Variant right = PopResult();

        Variant result;
        LangError error(LangError::Code::Success);

        if (!BinaryOp(binaryExpr.op, left, right, result, error))
        {
            ThrowError(error);
        }

        m_VariantStack.push(result);
    }

    void Interpreter::Visit(const NameExpr& nameExpr)
    {
        Variant val;
        if (!m_pVarStore->GetVar(nameExpr.name.value, val))
        {
            ThrowError(LangError(LangError::Code::Interpreter_UnableToResolveName,
                nameExpr.name.line, { nameExpr.name.value }));
        }

        m_VariantStack.push(val);
    }

    void Interpreter::Visit(const StringLiteralExpr& stringLiteralExpr)
    {
        m_VariantStack.push(Variant(Interpolate(stringLiteralExpr.value)));
    }

    void Interpreter::Visit(const NumberLiteralExpr& numberLiteralExpr)
    {
        m_VariantStack.push(Variant(numberLiteralExpr.value));
    }

    void Interpreter::Visit(const BoolLiteralExpr& boolLiteralExpr)
    {
        m_VariantStack.push(Variant(boolLiteralExpr.value));
    }

    Variant Interpreter::PopResult()
    {
        if (m_VariantStack.empty())
        {
            ThrowError(LangError(LangError::Code::InternalError));
            return Variant();
        }

        Variant result = m_VariantStack.top();
        m_VariantStack.pop();

        return result;
    }

    std::string Interpreter::Interpolate(const std::string& str)
    {
        return m_pVarStore->Interpolate(str);
    }

    void Interpreter::ThrowError(const LangError& error)
    {
        m_Error = error;
        throw std::runtime_error("");
    }

}