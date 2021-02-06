#pragma once

#include <vector>
#include <memory>

#include "hscpp/preprocessor/Token.h"

namespace hscpp
{
    struct Stmt;
    struct Expr;

    struct BlockStmt;
    struct IncludeStmt;
    struct HscppIfStmt;
    struct HscppReturnStmt;
    struct HscppRequireStmt;
    struct HscppModuleStmt;
    struct HscppMessageStmt;
    struct UnaryExpr;
    struct BinaryExpr;
    struct NameExpr;
    struct StringLiteralExpr;
    struct NumberLiteralExpr;
    struct BoolLiteralExpr;

    class IAstVisitor
    {
    public:
        virtual ~IAstVisitor() = default;

        virtual void Visit(const BlockStmt& blockStmt) = 0;
        virtual void Visit(const IncludeStmt& includeStmt) = 0;
        virtual void Visit(const HscppIfStmt& ifStmt) = 0;
        virtual void Visit(const HscppReturnStmt& ifStmt) = 0;
        virtual void Visit(const HscppRequireStmt& requireStmt) = 0;
        virtual void Visit(const HscppModuleStmt& moduleStmt) = 0;
        virtual void Visit(const HscppMessageStmt& messageStmt) = 0;
        virtual void Visit(const UnaryExpr& unaryExpr) = 0;
        virtual void Visit(const BinaryExpr& binaryExpr) = 0;
        virtual void Visit(const NameExpr& nameExpr) = 0;
        virtual void Visit(const StringLiteralExpr& stringLiteralExpr) = 0;
        virtual void Visit(const NumberLiteralExpr& numberLiteralExpr) = 0;
        virtual void Visit(const BoolLiteralExpr& boolLiteralExpr) = 0;
    };

    struct Stmt
    {
        virtual ~Stmt() = default;
        virtual void Accept(IAstVisitor& visitor) const = 0;
    };

    struct BlockStmt : public Stmt
    {
        void Accept(IAstVisitor& visitor) const override;

        std::vector<std::unique_ptr<Stmt>> statements;
    };

    struct IncludeStmt : public Stmt
    {
        void Accept(IAstVisitor& visitor) const override;

        std::string path;
    };

    struct HscppIfStmt : public Stmt
    {
        void Accept(IAstVisitor& visitor) const override;

        std::vector<std::unique_ptr<Expr>> conditions;
        std::vector<std::unique_ptr<BlockStmt>> conditionalBlocks;
        std::unique_ptr<BlockStmt> pElseBlock;
    };

    struct HscppReturnStmt : public Stmt
    {
        void Accept(IAstVisitor& visitor) const override;
    };

    struct HscppRequireStmt : public Stmt
    {
        void Accept(IAstVisitor& visitor) const override;

        Token token;
        std::vector<std::string> parameters;
    };

    struct HscppModuleStmt : public Stmt
    {
        void Accept(IAstVisitor& visitor) const override;

        std::string module;
    };

    struct HscppMessageStmt : public Stmt
    {
        void Accept(IAstVisitor& visitor) const override;

        std::string message;
    };

    struct Expr
    {
        virtual ~Expr() = default;
        virtual void Accept(IAstVisitor& visitor) const = 0;
    };

    struct NameExpr : public Expr
    {
        void Accept(IAstVisitor& visitor) const override;

        Token name;
    };

    struct StringLiteralExpr : public Expr
    {
        void Accept(IAstVisitor& visitor) const override;

        std::string value;
    };

    struct NumberLiteralExpr : public Expr
    {
        void Accept(IAstVisitor& visitor) const override;

        double value;
    };

    struct BoolLiteralExpr : public Expr
    {
        void Accept(IAstVisitor& visitor) const override;

        bool value;
    };

    struct UnaryExpr : public Expr
    {
        void Accept(IAstVisitor& visitor) const override;

        std::unique_ptr<Expr> pRightExpr;
        Token op;
    };

    struct BinaryExpr : public Expr
    {
        void Accept(IAstVisitor& visitor) const override;

        std::unique_ptr<Expr> pLeftExpr;
        std::unique_ptr<Expr> pRightExpr;
        Token op;
    };

}