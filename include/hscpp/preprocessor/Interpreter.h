#pragma once

#include <vector>
#include <stack>

#include "hscpp/preprocessor/Ast.h"
#include "hscpp/preprocessor/HscppRequire.h"
#include "hscpp/preprocessor/Variant.h"
#include "hscpp/preprocessor/LangError.h"
#include "hscpp/preprocessor/VarStore.h"

namespace hscpp
{

    class Interpreter : public IAstVisitor
    {
    public:
        struct Result
        {
            std::vector<HscppRequire> hscppRequires;
            std::vector<std::string> hscppModules;
            std::vector<std::string> hscppMessages;
            std::vector<std::string> includePaths;
        };

        bool Evaluate(const Stmt& rootStmt, const VarStore& varStore, Result& result);
        LangError GetLastError();

    private:
        // Thrown to immediately stop interpreter.
        struct ReturnFromInterpreter
        {};

        const VarStore* m_pVarStore = nullptr;
        Result* m_pResult = nullptr;

        LangError m_Error = LangError(LangError::Code::Success);

        std::stack<Variant> m_VariantStack;

        void Reset(const VarStore& varStore, Result& result);

        void Visit(const BlockStmt& blockStmt) override;
        void Visit(const IncludeStmt& includeStmt) override;
        void Visit(const HscppIfStmt& ifStmt) override;
        void Visit(const HscppReturnStmt& returnStmt) override;
        void Visit(const HscppRequireStmt& requireStmt) override;
        void Visit(const HscppModuleStmt& moduleStmt) override;
        void Visit(const HscppMessageStmt& messageStmt) override;
        void Visit(const UnaryExpr& unaryExpr) override;
        void Visit(const BinaryExpr& binaryExpr) override;
        void Visit(const NameExpr& nameExpr) override;
        void Visit(const StringLiteralExpr& stringLiteralExpr) override;
        void Visit(const NumberLiteralExpr& numberLiteralExpr) override;
        void Visit(const BoolLiteralExpr& boolLiteralExpr) override;

        Variant PopResult();
        std::string Interpolate(const std::string& str);

        void ThrowError(const LangError& error);
    };

}