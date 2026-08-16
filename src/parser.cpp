#include "parser.hpp"

#include <utility>

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

bool Parser::isAtEnd() const { return peek().kind == TokenKind::EndOfFile; }
const Token& Parser::peek() const { return tokens_[current_]; }
const Token& Parser::previous() const { return tokens_[current_ - 1]; }

const Token& Parser::advance() {
    if (!isAtEnd()) {
        ++current_;
    }
    return previous();
}

bool Parser::check(TokenKind kind) const {
    return !isAtEnd() && peek().kind == kind;
}

bool Parser::match(TokenKind kind) {
    if (check(kind)) {
        advance();
        return true;
    }
    return false;
}

const Token& Parser::consume(TokenKind kind, const std::string& message) {
    if (check(kind)) {
        return advance();
    }
    throw error(message);
}

Program Parser::parseProgram() {
    Program program;
    while (!isAtEnd()) {
        if (check(TokenKind::Struct) || check(TokenKind::Class)) {
            bool isClass = match(TokenKind::Class);
            if (!isClass) {
                consume(TokenKind::Struct, "expected 'struct'");
            }
            auto decl = parseStruct(isClass);
            structTypes_[decl->name] = decl.get();
            program.structs.push_back(std::move(decl));
        } else {
            program.functions.push_back(parseFunction());
        }
    }
    return program;
}

Type Parser::parseType() {
    Type base;
    if (match(TokenKind::Int)) {
        base = Type::makeInt();
    } else if (match(TokenKind::Bool)) {
        base = Type::makeBool();
    } else if (match(TokenKind::Char)) {
        base = Type::makeChar();
    } else if (match(TokenKind::Float)) {
        base = Type::makeFloat();
    } else if (match(TokenKind::Double)) {
        base = Type::makeDouble();
    } else if (match(TokenKind::Void)) {
        base = Type::makeVoid();
    } else if (check(TokenKind::Identifier)) {
        std::string name = advance().lexeme;
        auto it = structTypes_.find(name);
        if (it == structTypes_.end()) {
            throw error("unknown type '" + name + "'");
        }
        base = Type::makeStruct(name, it->second);
    } else {
        throw error("expected type");
    }

    while (true) {
        if (match(TokenKind::Star)) {
            base = Type::makePointer(base);
        } else if (match(TokenKind::Ampersand)) {
            base = Type::makeReference(base);
        } else if (match(TokenKind::LBracket)) {
            const Token& sizeToken = consume(TokenKind::Number, "expected array size");
            consume(TokenKind::RBracket, "expected ']' after array size");
            base = Type::makeArray(base, std::stoi(sizeToken.lexeme));
        } else {
            break;
        }
    }
    return base;
}

std::unique_ptr<StructDecl> Parser::parseStruct(bool isClass) {
    const Token& name = consume(TokenKind::Identifier, isClass ? "expected class name" : "expected struct name");
    consume(TokenKind::LBrace, "expected '{'");

    auto decl = std::make_unique<StructDecl>();
    decl->name = name.lexeme;
    structTypes_[decl->name] = decl.get();

    while (!check(TokenKind::RBrace) && !isAtEnd()) {
        if (match(TokenKind::Public) || match(TokenKind::Private) || match(TokenKind::Protected)) {
            consume(TokenKind::Colon, "expected ':' after access label");
            continue;
        }

        Type memberType = parseType();
        const Token& memberName = consume(TokenKind::Identifier, "expected member name");
        if (match(TokenKind::LParen)) {
            std::vector<std::pair<Type, std::string>> params;
            if (!check(TokenKind::RParen)) {
                do {
                    Type paramType = parseType();
                    const Token& paramName = consume(TokenKind::Identifier, "expected parameter name");
                    params.emplace_back(paramType, paramName.lexeme);
                } while (match(TokenKind::Comma));
            }
            consume(TokenKind::RParen, "expected ')' after parameters");
            auto body = parseBlock();

            auto fn = std::make_unique<FunctionDecl>();
            fn->returnType = memberType;
            fn->name = memberName.lexeme;
            fn->structName = decl->name;
            fn->params = std::move(params);
            fn->body = std::move(body);
            fn->isMethod = true;
            decl->methods.push_back(std::move(fn));
        } else {
            consume(TokenKind::Semicolon, "expected ';' after field");
            decl->fields.push_back(FieldDecl{memberType, memberName.lexeme, 0});
        }
    }

    consume(TokenKind::RBrace, "expected '}'");
    if (match(TokenKind::Semicolon)) {} // optional trailing semicolon
    structTypes_[decl->name] = decl.get();
    return decl;
}

std::unique_ptr<FunctionDecl> Parser::parseFunction(const std::string& structName) {
    Type returnType = parseType();
    const Token& name = consume(TokenKind::Identifier, "expected function name");
    consume(TokenKind::LParen, "expected '(' after function name");

    std::vector<std::pair<Type, std::string>> params;
    if (!check(TokenKind::RParen)) {
        do {
            Type paramType = parseType();
            const Token& paramName = consume(TokenKind::Identifier, "expected parameter name");
            params.emplace_back(paramType, paramName.lexeme);
        } while (match(TokenKind::Comma));
    }

    consume(TokenKind::RParen, "expected ')' after parameters");
    auto body = parseBlock();

    auto fn = std::make_unique<FunctionDecl>();
    fn->returnType = returnType;
    fn->name = name.lexeme;
    fn->structName = structName;
    fn->params = std::move(params);
    fn->body = std::move(body);
    fn->isMethod = !structName.empty();
    return fn;
}

std::unique_ptr<FunctionDecl> Parser::parseMethod(const std::string& structName) {
    return parseFunction(structName);
}

std::unique_ptr<BlockStmt> Parser::parseBlock() {
    consume(TokenKind::LBrace, "expected '{'");
    auto block = std::make_unique<BlockStmt>();
    while (!check(TokenKind::RBrace) && !isAtEnd()) {
        block->statements.push_back(parseStatement());
    }
    consume(TokenKind::RBrace, "expected '}'");
    return block;
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (match(TokenKind::If)) return parseIfStatement();
    if (match(TokenKind::While)) return parseWhileStatement();
    if (match(TokenKind::Do)) return parseDoWhileStatement();
    if (match(TokenKind::For)) return parseForStatement();
    if (match(TokenKind::Switch)) return parseSwitchStatement();
    if (match(TokenKind::Return)) return parseReturnStatement();
    if (match(TokenKind::Break)) return parseBreakStatement();
    if (match(TokenKind::Continue)) return parseContinueStatement();
    if (match(TokenKind::LBrace)) {
        --current_;
        return parseBlock();
    }
    if (check(TokenKind::Int) || check(TokenKind::Bool) || check(TokenKind::Char) ||
        check(TokenKind::Float) || check(TokenKind::Double) || check(TokenKind::Identifier)) {
        return parseVarDecl();
    }
    throw error("expected statement");
}

std::unique_ptr<Stmt> Parser::parseVarDecl() {
    if (check(TokenKind::Identifier) && !structTypes_.count(peek().lexeme)) {
        auto expr = parseAssignment();
        consume(TokenKind::Semicolon, "expected ';' after expression");
        return std::make_unique<ExprStmt>(std::move(expr));
    }

    Type type = parseType();
    const Token& name = consume(TokenKind::Identifier, "expected variable name");
    std::unique_ptr<Expr> initializer;
    if (match(TokenKind::Assign)) {
        initializer = parseAssignment();
    }
    consume(TokenKind::Semicolon, "expected ';' after variable declaration");
    return std::make_unique<VarDeclStmt>(type, name.lexeme, std::move(initializer));
}

std::unique_ptr<Stmt> Parser::parseIfStatement() {
    consume(TokenKind::LParen, "expected '(' after 'if'");
    auto condition = parseExpression();
    consume(TokenKind::RParen, "expected ')' after if condition");
    auto thenBranch = parseStatement();
    std::unique_ptr<Stmt> elseBranch;
    if (match(TokenKind::Else)) {
        elseBranch = parseStatement();
    }
    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::parseWhileStatement() {
    consume(TokenKind::LParen, "expected '(' after 'while'");
    auto condition = parseExpression();
    consume(TokenKind::RParen, "expected ')' after while condition");
    auto body = parseStatement();
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseDoWhileStatement() {
    auto body = parseStatement();
    consume(TokenKind::While, "expected 'while' after do body");
    consume(TokenKind::LParen, "expected '(' after 'while'");
    auto condition = parseExpression();
    consume(TokenKind::RParen, "expected ')' after while condition");
    consume(TokenKind::Semicolon, "expected ';' after do-while condition");
    return std::make_unique<DoWhileStmt>(std::move(body), std::move(condition));
}

std::unique_ptr<Stmt> Parser::parseForStatement() {
    consume(TokenKind::LParen, "expected '(' after 'for'");
    std::unique_ptr<Stmt> initializer;
    if (!check(TokenKind::Semicolon)) {
        initializer = parseVarDecl();
    } else {
        advance();
    }

    std::unique_ptr<Expr> condition;
    if (!check(TokenKind::Semicolon)) {
        condition = parseExpression();
    }
    consume(TokenKind::Semicolon, "expected ';' after for condition");

    std::unique_ptr<Expr> increment;
    if (!check(TokenKind::RParen)) {
        increment = parseAssignment();
    }
    consume(TokenKind::RParen, "expected ')' after for clauses");
    auto body = parseStatement();
    return std::make_unique<ForStmt>(std::move(initializer), std::move(condition),
                                     std::move(increment), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseSwitchStatement() {
    consume(TokenKind::LParen, "expected '(' after 'switch'");
    auto condition = parseExpression();
    consume(TokenKind::RParen, "expected ')' after switch condition");
    consume(TokenKind::LBrace, "expected '{' after switch condition");

    std::vector<CaseBranch> cases;
    while (!check(TokenKind::RBrace) && !isAtEnd()) {
        if (match(TokenKind::Case)) {
            const Token& valTok = consume(TokenKind::Number, "expected integer constant for case");
            consume(TokenKind::Colon, "expected ':' after case constant");
            CaseBranch cb;
            cb.value = std::stoi(valTok.lexeme);
            cb.isDefault = false;
            while (!check(TokenKind::Case) && !check(TokenKind::Default) && !check(TokenKind::RBrace) && !isAtEnd()) {
                cb.statements.push_back(parseStatement());
            }
            cases.push_back(std::move(cb));
        } else if (match(TokenKind::Default)) {
            consume(TokenKind::Colon, "expected ':' after default");
            CaseBranch cb;
            cb.isDefault = true;
            while (!check(TokenKind::Case) && !check(TokenKind::Default) && !check(TokenKind::RBrace) && !isAtEnd()) {
                cb.statements.push_back(parseStatement());
            }
            cases.push_back(std::move(cb));
        } else {
            advance();
        }
    }
    consume(TokenKind::RBrace, "expected '}' after switch body");
    return std::make_unique<SwitchStmt>(std::move(condition), std::move(cases));
}

std::unique_ptr<Stmt> Parser::parseReturnStatement() {
    std::unique_ptr<Expr> value;
    if (!check(TokenKind::Semicolon)) {
        value = parseExpression();
    }
    consume(TokenKind::Semicolon, "expected ';' after return value");
    return std::make_unique<ReturnStmt>(std::move(value));
}

std::unique_ptr<Stmt> Parser::parseBreakStatement() {
    consume(TokenKind::Semicolon, "expected ';' after break");
    return std::make_unique<BreakStmt>();
}

std::unique_ptr<Stmt> Parser::parseContinueStatement() {
    consume(TokenKind::Semicolon, "expected ';' after continue");
    return std::make_unique<ContinueStmt>();
}

std::unique_ptr<Expr> Parser::parseExpression() { return parseAssignment(); }

std::unique_ptr<Expr> Parser::parseAssignment() {
    auto expr = parseTernary();
    if (match(TokenKind::Assign)) {
        auto value = parseAssignment();
        return std::make_unique<AssignExpr>(std::move(expr), std::move(value));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::parseTernary() {
    auto expr = parseLogicalOr();
    if (match(TokenKind::Question)) {
        auto thenExpr = parseExpression();
        consume(TokenKind::Colon, "expected ':' in ternary expression");
        auto elseExpr = parseTernary();
        return std::make_unique<TernaryExpr>(std::move(expr), std::move(thenExpr), std::move(elseExpr));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::parseLogicalOr() {
    auto expr = parseLogicalAnd();
    while (match(TokenKind::OrOr)) {
        auto right = parseLogicalAnd();
        expr = std::make_unique<BinaryExpr>('|', std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::parseLogicalAnd() {
    auto expr = parseEquality();
    while (match(TokenKind::AndAnd)) {
        auto right = parseEquality();
        expr = std::make_unique<BinaryExpr>('&', std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::parseEquality() {
    auto expr = parseComparison();
    while (true) {
        if (match(TokenKind::Eq)) {
            expr = std::make_unique<BinaryExpr>('=', std::move(expr), parseComparison());
        } else if (match(TokenKind::Neq)) {
            expr = std::make_unique<BinaryExpr>('!', std::move(expr), parseComparison());
        } else {
            break;
        }
    }
    return expr;
}

std::unique_ptr<Expr> Parser::parseComparison() {
    auto expr = parseTerm();
    while (true) {
        if (match(TokenKind::Lt)) {
            expr = std::make_unique<BinaryExpr>('<', std::move(expr), parseTerm());
        } else if (match(TokenKind::Gt)) {
            expr = std::make_unique<BinaryExpr>('>', std::move(expr), parseTerm());
        } else if (match(TokenKind::Le)) {
            expr = std::make_unique<BinaryExpr>('l', std::move(expr), parseTerm());
        } else if (match(TokenKind::Ge)) {
            expr = std::make_unique<BinaryExpr>('g', std::move(expr), parseTerm());
        } else {
            break;
        }
    }
    return expr;
}

std::unique_ptr<Expr> Parser::parseTerm() {
    auto expr = parseFactor();
    while (true) {
        if (match(TokenKind::Plus)) {
            expr = std::make_unique<BinaryExpr>('+', std::move(expr), parseFactor());
        } else if (match(TokenKind::Minus)) {
            expr = std::make_unique<BinaryExpr>('-', std::move(expr), parseFactor());
        } else {
            break;
        }
    }
    return expr;
}

std::unique_ptr<Expr> Parser::parseFactor() {
    auto expr = parseUnary();
    while (true) {
        if (match(TokenKind::Star)) {
            expr = std::make_unique<BinaryExpr>('*', std::move(expr), parseUnary());
        } else if (match(TokenKind::Slash)) {
            expr = std::make_unique<BinaryExpr>('/', std::move(expr), parseUnary());
        } else if (match(TokenKind::Percent)) {
            expr = std::make_unique<BinaryExpr>('%', std::move(expr), parseUnary());
        } else {
            break;
        }
    }
    return expr;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (match(TokenKind::Minus)) {
        return std::make_unique<UnaryExpr>('-', parseUnary());
    }
    if (match(TokenKind::Bang)) {
        return std::make_unique<UnaryExpr>('!', parseUnary());
    }
    if (match(TokenKind::Star)) {
        return std::make_unique<UnaryExpr>('*', parseUnary());
    }
    if (match(TokenKind::Ampersand)) {
        return std::make_unique<UnaryExpr>('&', parseUnary());
    }
    return parsePostfix();
}

std::unique_ptr<Expr> Parser::parsePostfix() {
    auto expr = parsePrimary();
    while (true) {
        if (match(TokenKind::LBracket)) {
            auto index = parseExpression();
            consume(TokenKind::RBracket, "expected ']' after index");
            expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index));
        } else if (match(TokenKind::Dot)) {
            const Token& member = consume(TokenKind::Identifier, "expected member name");
            if (match(TokenKind::LParen)) {
                std::vector<std::unique_ptr<Expr>> args;
                if (!check(TokenKind::RParen)) {
                    do {
                        args.push_back(parseExpression());
                    } while (match(TokenKind::Comma));
                }
                consume(TokenKind::RParen, "expected ')' after arguments");
                expr = std::make_unique<MethodCallExpr>(std::move(expr), member.lexeme, std::move(args));
            } else {
                expr = std::make_unique<MemberExpr>(std::move(expr), member.lexeme);
            }
        } else if (match(TokenKind::Arrow)) {
            const Token& member = consume(TokenKind::Identifier, "expected member name");
            if (match(TokenKind::LParen)) {
                std::vector<std::unique_ptr<Expr>> args;
                if (!check(TokenKind::RParen)) {
                    do {
                        args.push_back(parseExpression());
                    } while (match(TokenKind::Comma));
                }
                consume(TokenKind::RParen, "expected ')' after arguments");
                expr = std::make_unique<MethodCallExpr>(std::move(expr), member.lexeme, std::move(args));
            } else {
                expr = std::make_unique<MemberExpr>(std::move(expr), member.lexeme);
            }
        } else if (match(TokenKind::LParen)) {
            if (auto* var = dynamic_cast<VarExpr*>(expr.get())) {
                std::vector<std::unique_ptr<Expr>> args;
                if (!check(TokenKind::RParen)) {
                    do {
                        args.push_back(parseExpression());
                    } while (match(TokenKind::Comma));
                }
                consume(TokenKind::RParen, "expected ')' after arguments");
                expr = std::make_unique<CallExpr>(var->name, std::move(args));
            } else {
                throw error("only identifier calls supported in this position");
            }
        } else {
            break;
        }
    }
    return expr;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    if (match(TokenKind::Number)) {
        return std::make_unique<IntLiteral>(std::stoi(previous().lexeme));
    }
    if (match(TokenKind::FloatLiteral)) {
        return std::make_unique<FloatLiteral>(std::stod(previous().lexeme));
    }
    if (match(TokenKind::CharLiteral)) {
        char c = previous().lexeme.empty() ? '\0' : previous().lexeme[0];
        return std::make_unique<CharLiteral>(c);
    }
    if (match(TokenKind::StringLiteral)) {
        return std::make_unique<StringLiteral>(previous().lexeme);
    }
    if (match(TokenKind::True)) {
        return std::make_unique<BoolLiteral>(true);
    }
    if (match(TokenKind::False)) {
        return std::make_unique<BoolLiteral>(false);
    }
    if (match(TokenKind::Identifier)) {
        return std::make_unique<VarExpr>(previous().lexeme);
    }
    if (match(TokenKind::LParen)) {
        auto expr = parseExpression();
        consume(TokenKind::RParen, "expected ')' after expression");
        return expr;
    }
    throw error("expected expression");
}

ParseError Parser::error(const std::string& message) const {
    const Token& token = peek();
    return ParseError("Parse error at line " + std::to_string(token.line) + ", column " +
                      std::to_string(token.column) + ": " + message);
}
