#pragma once

#include "token.hpp"

#include <string>
#include <vector>

class Lexer {
public:
    explicit Lexer(std::string source);

    std::vector<Token> tokenize();

private:
    std::string source_;
    size_t pos_ = 0;
    int line_ = 1;
    int column_ = 1;

    char peek(size_t offset = 0) const;
    char advance();
    bool match(char expected);
    void skipWhitespaceAndComments();
    Token makeToken(TokenKind kind, const std::string& lexeme);
    Token number();
    Token stringLiteral();
    Token charLiteral();
    Token identifier();
    Token invalid(const std::string& message);
};
