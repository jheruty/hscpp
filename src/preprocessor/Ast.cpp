#include "hscpp/preprocessor/Ast.h"

namespace hscpp
{

    void BlockStmt::Accept(IAstVisitor& visitor) const
    {
        visitor.Visit(*this);
    }

    void IncludeStmt::Accept(IAstVisitor& visitor) const
    {
        visitor.Visit(*this);
    }

    void HscppIfStmt::Accept(IAstVisitor& visitor) const
    {
        visitor.Visit(*this);
    }

    void HscppReturnStmt::Accept(IAstVisitor& visitor) const
    {
        visitor.Visit(*this);
    }

    void HscppRequireStmt::Accept(IAstVisitor& visitor) const
    {
        visitor.Visit(*this);
    }

    void HscppModuleStmt::Accept(IAstVisitor& visitor) const
    {
        visitor.Visit(*this);
    }

    void HscppMessageStmt::Accept(IAstVisitor& visitor) const
    {
        visitor.Visit(*this);
    }

    void NameExpr::Accept(IAstVisitor& visitor) const
    {
        visitor.Visit(*this);
    }

    void StringLiteralExpr::Accept(IAstVisitor& visitor) const
    {
        visitor.Visit(*this);
    }

    void NumberLiteralExpr::Accept(IAstVisitor& visitor) const
    {
        visitor.Visit(*this);
    }

    void BoolLiteralExpr::Accept(IAstVisitor& visitor) const
    {
        visitor.Visit(*this);
    }

    void UnaryExpr::Accept(IAstVisitor& visitor) const
    {
        visitor.Visit(*this);
    }

    void BinaryExpr::Accept(IAstVisitor& visitor) const
    {
        visitor.Visit(*this);
    }

}