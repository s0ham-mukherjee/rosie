#pragma once

#include "ast.hpp"
#include "token.hpp"

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& message) : std::runtime_error(message) {}
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    Program parseProgram();

private:
    std::vector<Token> tokens_;
    size_t current_ = 0;
    std::unordered_map<std::string, StructDecl*> structTypes_;

    bool isAtEnd() const;
    const Token& peek() const;
    const Token& previous() const;
    const Token& advance();
    bool check(TokenKind kind) const;
    bool match(TokenKind kind);
    const Token& consume(TokenKind kind, const std::string& message);

    std::unique_ptr<StructDecl> parseStruct(bool isClass);
    std::unique_ptr<FunctionDecl> parseFunction(const std::string& structName = "");
    std::unique_ptr<FunctionDecl> parseMethod(const std::string& structName);
    std::unique_ptr<BlockStmt> parseBlock();
    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Stmt> parseVarDecl();
    std::unique_ptr<Stmt> parseIfStatement();
    std::unique_ptr<Stmt> parseWhileStatement();
    std::unique_ptr<Stmt> parseDoWhileStatement();
    std::unique_ptr<Stmt> parseForStatement();
    std::unique_ptr<Stmt> parseSwitchStatement();
    std::unique_ptr<Stmt> parseReturnStatement();
    std::unique_ptr<Stmt> parseBreakStatement();
    std::unique_ptr<Stmt> parseContinueStatement();

    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parseAssignment();
    std::unique_ptr<Expr> parseTernary();
    std::unique_ptr<Expr> parseLogicalOr();
    std::unique_ptr<Expr> parseLogicalAnd();
    std::unique_ptr<Expr> parseEquality();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseTerm();
    std::unique_ptr<Expr> parseFactor();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePostfix();
    std::unique_ptr<Expr> parsePrimary();

    Type parseType();
    ParseError error(const std::string& message) const;
};
