#include <iostream>
#include <cassert>

#include "hscpp/preprocessor/Parser.h"

// Shout out to Bob Nystrom for his book "Crafting Interpreters"!

namespace hscpp
{

    bool Parser::Parse(const std::vector<Token>& tokens, std::unique_ptr<Stmt>& pRootStmt)
    {
        Reset(tokens);
        try
        {
            pRootStmt = ParseBlockStmt();
            return true;
        }
        catch (const std::runtime_error&)
        {
            return false;
        }
    }

    LangError Parser::GetLastError()
    {
        return m_Error;
    }

    void Parser::Reset(const std::vector<Token>& tokens)
    {
        m_pTokens = &tokens;
        m_iToken = 0;

        m_Scopes = std::stack<std::unique_ptr<BlockStmt>>();

        // Last token contains the highest line number in the program. If Peek() returns the default
        // token, make the line number be set to the end of the program.
        m_DefaultToken = Token();
        if (!tokens.empty())
        {
            m_DefaultToken.line = tokens.back().line;
        }
    }

    std::unique_ptr<Expr> Parser::ParseExpr(int precedence /* = 0 */)
    {
        auto pExpr = ParsePrefixExpr();
        while (precedence < GetInfixPrecedence())
        {
            pExpr = ParseInfixExpr(std::move(pExpr));
        }

        return pExpr;
    }

    std::unique_ptr<Expr> Parser::ParsePrefixExpr()
    {
        switch (Peek().type)
        {
            case Token::Type::String:
                return ParseStringLiteralExpr();
            case Token::Type::Number:
                return ParseNumberLiteralExpr();
            case Token::Type::Bool:
                return ParseBoolLiteralExpr();
            case Token::Type::LeftParen:
                return ParseGroupExpr();
            case Token::Type::Identifier:
                return ParseNameExpr();
            case Token::Type::Minus:
            case Token::Type::Exclamation:
                return ParseUnaryExpr();
            default:
                ThrowError(LangError(LangError::Code::Parser_FailedToParsePrefixExpression,
                        Peek().line, { Peek().value }));
                break;
        }

        return nullptr;
    }

    std::unique_ptr<Expr> Parser::ParseInfixExpr(std::unique_ptr<Expr> pLeftExpr)
    {
        switch (Peek().type)
        {
            case Token::Type::Equivalent:
            case Token::Type::Inequivalent:
            case Token::Type::LessThan:
            case Token::Type::LessThanOrEqual:
            case Token::Type::GreaterThan:
            case Token::Type::GreaterThanOrEqual:
            case Token::Type::LogicalAnd:
            case Token::Type::LogicalOr:
            case Token::Type::Plus:
            case Token::Type::Minus:
            case Token::Type::Slash:
            case Token::Type::Star:
                return ParseBinaryExpr(std::move(pLeftExpr));
            default:
                ThrowError(LangError(LangError::Code::Parser_FailedToParseInfixExpression,
                        Peek().line, { Peek().value }));
                break;
        }

        return nullptr;
    }

    std::unique_ptr<Expr> Parser::ParseStringLiteralExpr()
    {
        auto pStringLiteralExpr = std::unique_ptr<StringLiteralExpr>(new StringLiteralExpr());
        pStringLiteralExpr->value = Peek().value;
        Consume(); // string

        return pStringLiteralExpr;
    }

    std::unique_ptr<Expr> Parser::ParseNumberLiteralExpr()
    {
        auto pNumberLiteralExpr = std::unique_ptr<NumberLiteralExpr>(new NumberLiteralExpr());

        try
        {
            pNumberLiteralExpr->value = std::stod(Peek().value);
            Consume(); // number
        }
        catch (const std::invalid_argument&)
        {
            ThrowError(LangError(LangError::Code::Parser_FailedToParseNumber,
                    Peek().line, { Peek().value }));
        }
        catch (const std::out_of_range&)
        {
            ThrowError(LangError(LangError::Code::Parser_NumberIsOutOfRange,
                    Peek().line, { Peek().value }));
        }

        return pNumberLiteralExpr;
    }

    std::unique_ptr<Expr> Parser::ParseBoolLiteralExpr()
    {
        auto pBoolLiteralExpr = std::unique_ptr<BoolLiteralExpr>(new BoolLiteralExpr());

        if (Peek().value == "true")
        {
            pBoolLiteralExpr->value = true;
            Consume(); // true
        }
        else if (Peek().value == "false")
        {
            pBoolLiteralExpr->value = false;
            Consume(); // false
        }
        else
        {
            // This should never happen...
            assert(false);
            ThrowError(LangError(LangError::Code::InternalError));
        }

        return pBoolLiteralExpr;
    }

    std::unique_ptr<Expr> Parser::ParseGroupExpr()
    {
        Consume(); // '('

        auto pExpr = ParseExpr();
        Expect(Token::Type::RightParen, LangError(LangError::Code::Parser_GroupExpressionMissingClosingParen,
                Prev().line, Prev().column, {}));
        Consume(); // ')'

        return pExpr;
    }

    std::unique_ptr<Expr> Parser::ParseNameExpr()
    {
        auto pNameExpr = std::unique_ptr<NameExpr>(new NameExpr());
        pNameExpr->name = Peek();
        Consume(); // identifier

        return pNameExpr;
    }

    std::unique_ptr<Expr> Parser::ParseUnaryExpr()
    {
        auto pUnaryExpr = std::unique_ptr<UnaryExpr>(new UnaryExpr());

        int precedence = GetPrefixPrecedence();
        pUnaryExpr->op = Peek();
        Consume(); // -, etc...

        pUnaryExpr->pRightExpr = ParseExpr(precedence);

        return pUnaryExpr;
    }

    std::unique_ptr<Expr> Parser::ParseBinaryExpr(std::unique_ptr<Expr> pLeftExpr)
    {
        auto pBinaryExpr = std::unique_ptr<BinaryExpr>(new BinaryExpr());

        int precedence = GetInfixPrecedence();
        pBinaryExpr->op = Peek();
        Consume(); // ==, <, >, etc...

        pBinaryExpr->pLeftExpr = std::move(pLeftExpr);
        pBinaryExpr->pRightExpr = ParseExpr(precedence);

        return pBinaryExpr;
    }

    int Parser::GetPrefixPrecedence()
    {
        return 7;
    }

    int Parser::GetInfixPrecedence()
    {
        switch (Peek().type)
        {
            case Token::Type::LogicalOr:
                return 1;
            case Token::Type::LogicalAnd:
                return 2;
            case Token::Type::Equivalent:
            case Token::Type::Inequivalent:
                return 3;
            case Token::Type::LessThan:
            case Token::Type::LessThanOrEqual:
            case Token::Type::GreaterThan:
            case Token::Type::GreaterThanOrEqual:
                return 4;
            case Token::Type::Minus:
            case Token::Type::Plus:
                return 5;
            case Token::Type::Slash:
            case Token::Type::Star:
                return 6;
            case Token::Type::Unknown:
                // Unhandled infix operator, always evaluate it to force an error.
                return (std::numeric_limits<int>::max)();
            default:
                // Not an infix operator.
                return -1;
        }

        return -1;
    }

    std::unique_ptr<BlockStmt> Parser::ParseBlockStmt()
    {
        auto pBlockStmt = std::unique_ptr<BlockStmt>(new BlockStmt());

        while (!IsAtEnd())
        {
            switch (Peek().type)
            {
                case Token::Type::Include:
                    pBlockStmt->statements.push_back(ParseIncludeStmt());
                    break;
                case Token::Type::HscppIf:
                    pBlockStmt->statements.push_back(ParseHscppIfStmt());
                    break;
                case Token::Type::HscppReturn:
                    pBlockStmt->statements.push_back(ParseHscppReturnStmt());
                    break;
                case Token::Type::HscppElif:
                case Token::Type::HscppElse:
                case Token::Type::HscppEnd:
                    return pBlockStmt;
                case Token::Type::HscppRequireSource:
                case Token::Type::HscppRequireIncludeDir:
                case Token::Type::HscppRequireLibrary:
                case Token::Type::HscppRequireLibraryDir:
                case Token::Type::HscppRequirePreprocessorDef:
                    pBlockStmt->statements.push_back(ParseHscppRequireStmt());
                    break;
                case Token::Type::HscppModule:
                case Token::Type::HscppTrack:
                    pBlockStmt->statements.push_back(ParseHscppModuleStmt());
                    break;
                case Token::Type::HscppMessage:
                    pBlockStmt->statements.push_back(ParseHscppMessageStmt());
                    break;
                default:
                    Consume();
                    break;
            }
        }

        return pBlockStmt;
    }

    std::unique_ptr<Stmt> Parser::ParseIncludeStmt()
    {
        auto pInclude = std::unique_ptr<IncludeStmt>(new IncludeStmt());

        Consume(); // include

        Expect(Token::Type::String, LangError(LangError::Code::Parser_IncludeMissingPath, Prev().line, {}));
        pInclude->path = Peek().value;
        Consume(); // string

        return pInclude;
    }

    std::unique_ptr<Stmt> Parser::ParseHscppIfStmt()
    {
        auto pIf = std::unique_ptr<HscppIfStmt>(new HscppIfStmt());

        std::string ifStmtName;
        while (!IsAtEnd())
        {
            ifStmtName = Peek().value;

            if (Peek().type == Token::Type::HscppIf || Peek().type == Token::Type::HscppElif)
            {
                Consume(); // hscpp_if || hscpp_elif

                Expect(Token::Type::LeftParen, LangError(LangError::Code::Parser_HscppStmtMissingOpeningParen,
                        Peek().line, { ifStmtName }));
                Consume(); // '(

                pIf->conditions.push_back(ParseExpr());

                Expect(Token::Type::RightParen, LangError(LangError::Code::Parser_HscppStmtMissingClosingParen,
                        Peek().line, { ifStmtName }));
                Consume(); // ')'

                pIf->conditionalBlocks.push_back(ParseBlockStmt());
            }
            else if (Peek().type == Token::Type::HscppElse)
            {
                Consume(); // hscpp_else

                Expect(Token::Type::LeftParen, LangError(LangError::Code::Parser_HscppStmtMissingOpeningParen,
                        Peek().line, { ifStmtName }));
                Consume(); // '(

                Expect(Token::Type::RightParen, LangError(LangError::Code::Parser_HscppStmtMissingClosingParen,
                        Peek().line, { ifStmtName }));
                Consume(); // ')'

                pIf->pElseBlock = ParseBlockStmt();
            }
            else
            {
                break;
            }
        }

        Expect(Token::Type::HscppEnd, LangError(LangError::Code::Parser_HscppIfStmtMissingHscppEnd,
                Peek().line, { ifStmtName }));
        Consume(); // hscpp_end

        Expect(Token::Type::LeftParen, LangError(LangError::Code::Parser_HscppStmtMissingOpeningParen,
                Peek().line, { "hscpp_end" }));
        Consume(); // '(

        Expect(Token::Type::RightParen, LangError(LangError::Code::Parser_HscppStmtMissingClosingParen,
                Peek().line, { "hscpp_end" }));
        Consume(); // ')'

        return pIf;
    }

    std::unique_ptr<Stmt> Parser::ParseHscppReturnStmt()
    {
        auto pReturn = std::unique_ptr<HscppReturnStmt>(new HscppReturnStmt());

        Consume(); // hscpp_return

        Expect(Token::Type::LeftParen, LangError(LangError::Code::Parser_HscppStmtMissingOpeningParen,
            Peek().line, { "hscpp_return" }));
        Consume(); // '('

        Expect(Token::Type::RightParen, LangError(LangError::Code::Parser_HscppStmtMissingClosingParen,
            Peek().line, { "hscpp_return" }));
        Consume(); // ')'

        return pReturn;
    }

    std::unique_ptr<Stmt> Parser::ParseHscppRequireStmt()
    {
        auto pRequire = std::unique_ptr<HscppRequireStmt>(new HscppRequireStmt());
        pRequire->token = Peek();

        Consume(); // hscpp_require_*

        Expect(Token::Type::LeftParen, LangError(LangError::Code::Parser_HscppStmtMissingOpeningParen,
                Peek().line, { pRequire->token.value }));
        Consume(); // '('

        while (!IsAtEnd())
        {
            if (pRequire->token.type == Token::Type::HscppRequirePreprocessorDef)
            {
                // hscpp_require_preprocessor_def also allows identifiers.
                if (Peek().type != Token::Type::String && Peek().type != Token::Type::Identifier)
                {
                    ThrowError(LangError(
                            LangError::Code::Parser_HscppStmtExpectedStringLiteralOrIdentifierInArgumentList,
                            Peek().line, { pRequire->token.value }));
                }
            }
            else
            {
                Expect(Token::Type::String,
                        LangError(LangError::Code::Parser_HscppStmtExpectedStringLiteralInArgumentList,
                        Peek().line, { pRequire->token.value }));
            }

            pRequire->parameters.push_back(Peek().value);
            Consume(); // string || identifier

            if (Peek().type == Token::Type::RightParen)
            {
                break;
            }
            else
            {
                Expect(Token::Type::Comma, LangError(LangError::Code::Parser_HscppStmtMissingCommaInArgumentList,
                        Peek().line, Peek().column, { pRequire->token.value }));
                Consume(); // ','
            }
        }

        Expect(Token::Type::RightParen, LangError(LangError::Code::Parser_HscppStmtMissingClosingParen,
                Peek().line, { pRequire->token.value }));
        Consume(); // ')'

        return pRequire;
    }

    std::unique_ptr<Stmt> Parser::ParseHscppModuleStmt()
    {
        auto pModule = std::unique_ptr<HscppModuleStmt>(new HscppModuleStmt());

        if (Peek().type == Token::Type::HscppModule)
        {
            Consume(); // hscpp_module

            Expect(Token::Type::LeftParen, LangError(LangError::Code::Parser_HscppStmtMissingOpeningParen,
                Peek().line, { "hscpp_module" }));
            Consume(); // '('

            Expect(Token::Type::String, LangError(LangError::Code::Parser_HscppStmtArgumentMustBeStringLiteral,
                Peek().line, { "hscpp_module" }));
            pModule->module = Peek().value;
            Consume(); // string

            Expect(Token::Type::RightParen, LangError(LangError::Code::Parser_HscppStmtMissingClosingParen,
                Peek().line, { "hscpp_module" }));
            Consume(); // ')'
        }
        else if (Peek().type == Token::Type::HscppTrack)
        {
            // Treat HSCPP_TRACK as a module, so that layout changes to runtime classes forces
            // a recompilation of dependents.
            Consume(); // HSCPP_TRACK

            Expect(Token::Type::LeftParen, LangError(LangError::Code::Parser_HscppStmtMissingOpeningParen,
                Peek().line, { "HSCPP_TRACK" }));
            Consume(); // '('

            Expect(Token::Type::Identifier, LangError(LangError::Code::Parser_HscppTrackMissingIdentifier,
                Peek().line, {}));
            Consume(); // identifier

            Expect(Token::Type::Comma, LangError(LangError::Code::Parser_HscppStmtMissingCommaInArgumentList,
                Peek().line, Peek().column, { "HSCPP_TRACK" }));
            Consume(); // ','

            // The HSCPP_TRACK string is expected to be globally unique, and makes a reasonable
            // candidate for an autogenerated module name.
            Expect(Token::Type::String, LangError(LangError::Code::Parser_HscppTrackMissingString,
                Peek().line, {}));
            pModule->module = "@" + Peek().value;
            Consume(); // identifier

            Expect(Token::Type::RightParen, LangError(LangError::Code::Parser_HscppStmtMissingClosingParen,
                Peek().line, { "HSCPP_TRACK" }));
            Consume(); // ')'
        }
        else
        {
            // This should never happen...
            assert(false);
            ThrowError(LangError(LangError::Code::InternalError));
        }

        return pModule;
    }

    std::unique_ptr<Stmt> Parser::ParseHscppMessageStmt()
    {
        auto pMessage = std::unique_ptr<HscppMessageStmt>(new HscppMessageStmt());

        Consume(); // hscpp_module

        Expect(Token::Type::LeftParen, LangError(LangError::Code::Parser_HscppStmtMissingOpeningParen,
                Peek().line, { "hscpp_message" }));
        Consume(); // '('

        Expect(Token::Type::String, LangError(LangError::Code::Parser_HscppStmtArgumentMustBeStringLiteral,
                Peek().line, { "hscpp_message" }));
        pMessage->message = Peek().value;
        Consume(); // string

        Expect(Token::Type::RightParen, LangError(LangError::Code::Parser_HscppStmtMissingClosingParen,
                Peek().line, { "hscpp_message" }));
        Consume(); // ')'

        return pMessage;
    }

    const Token& Parser::Peek()
    {
        if (m_iToken < m_pTokens->size())
        {
            return m_pTokens->at(m_iToken);
        }

        return m_DefaultToken;
    }

    const Token& Parser::Prev()
    {
        if (m_iToken > 0 && m_iToken <= m_pTokens->size())
        {
            return m_pTokens->at(m_iToken - 1);
        }

        return m_DefaultToken;
    }

    void Parser::Consume()
    {
        ++m_iToken;
    }

    bool Parser::IsAtEnd()
    {
        return m_iToken >= m_pTokens->size();
    }

    void Parser::ThrowError(const LangError& error)
    {
        m_Error = error;
        throw std::runtime_error("");
    }

    void Parser::Expect(Token::Type tokenType, const LangError& error)
    {
        if (tokenType != Peek().type)
        {
            ThrowError(error);
        }
    }

}