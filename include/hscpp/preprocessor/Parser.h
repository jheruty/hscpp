#pragma once

#include <memory>
#include <stack>

#include "hscpp/preprocessor/Token.h"
#include "hscpp/preprocessor/Ast.h"
#include "hscpp/preprocessor/LangError.h"

namespace hscpp
{
    class Parser
    {
    public:
        bool Parse(const std::vector<Token>& tokens, std::unique_ptr<Stmt>& pRootStmt);
        LangError GetLastError();

    private:
        std::stack<std::unique_ptr<BlockStmt>> m_Scopes;

        const std::vector<Token>* m_pTokens = nullptr;
        size_t m_iToken = 0;

        LangError m_Error = LangError(LangError::Code::Success);

        Token m_DefaultToken;

        void Reset(const std::vector<Token>& tokens);

        std::unique_ptr<Expr> ParseExpr(int precedence = 0);
        std::unique_ptr<Expr> ParsePrefixExpr();
        std::unique_ptr<Expr> ParseInfixExpr(std::unique_ptr<Expr> pLeftExpr);

        std::unique_ptr<Expr> ParseStringLiteralExpr();
        std::unique_ptr<Expr> ParseNumberLiteralExpr();
        std::unique_ptr<Expr> ParseBoolLiteralExpr();
        std::unique_ptr<Expr> ParseGroupExpr();
        std::unique_ptr<Expr> ParseNameExpr();
        std::unique_ptr<Expr> ParseUnaryExpr();
        std::unique_ptr<Expr> ParseBinaryExpr(std::unique_ptr<Expr> pLeftExpr);

        int GetPrefixPrecedence();
        int GetInfixPrecedence();

        std::unique_ptr<BlockStmt> ParseBlockStmt();
        std::unique_ptr<Stmt> ParseIncludeStmt();
        std::unique_ptr<Stmt> ParseHscppIfStmt();
        std::unique_ptr<Stmt> ParseHscppReturnStmt();
        std::unique_ptr<Stmt> ParseHscppRequireStmt();
        std::unique_ptr<Stmt> ParseHscppModuleStmt();
        std::unique_ptr<Stmt> ParseHscppMessageStmt();

        const Token& Peek();
        const Token& Prev();
        void Consume();

        bool IsAtEnd();
        void ThrowError(const LangError& error);
        void Expect(Token::Type tokenType, const LangError& error);
    };
}