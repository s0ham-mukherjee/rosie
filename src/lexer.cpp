#include "lexer.hpp"

#include <cctype>
#include <unordered_map>

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

char Lexer::peek(size_t offset) const {
    if (pos_ + offset >= source_.size()) {
        return '\0';
    }
    return source_[pos_ + offset];
}

char Lexer::advance() {
    char c = peek();
    if (c == '\0') {
        return c;
    }
    ++pos_;
    if (c == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }
    return c;
}

bool Lexer::match(char expected) {
    if (peek() != expected) {
        return false;
    }
    advance();
    return true;
}

void Lexer::skipWhitespaceAndComments() {
    while (true) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
            continue;
        }
        if (c == '/' && peek(1) == '/') {
            while (peek() != '\0' && peek() != '\n') {
                advance();
            }
            continue;
        }
        if (c == '/' && peek(1) == '*') {
            advance(); // consume '/'
            advance(); // consume '*'
            while (peek() != '\0') {
                if (peek() == '*' && peek(1) == '/') {
                    advance();
                    advance();
                    break;
                }
                advance();
            }
            continue;
        }
        break;
    }
}

Token Lexer::makeToken(TokenKind kind, const std::string& lexeme) {
    return Token{kind, lexeme, line_, column_};
}

Token Lexer::number() {
    int startLine = line_;
    int startColumn = column_;
    std::string lexeme;
    bool isFloat = false;

    while (std::isdigit(static_cast<unsigned char>(peek()))) {
        lexeme += advance();
    }

    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peek(1)))) {
        isFloat = true;
        lexeme += advance(); // '.'
        while (std::isdigit(static_cast<unsigned char>(peek()))) {
            lexeme += advance();
        }
    }

    TokenKind kind = isFloat ? TokenKind::FloatLiteral : TokenKind::Number;
    return Token{kind, lexeme, startLine, startColumn};
}

Token Lexer::stringLiteral() {
    int startLine = line_;
    int startColumn = column_;
    advance(); // consume opening quote '"'
    std::string lexeme;

    while (peek() != '"' && peek() != '\0') {
        if (peek() == '\\') {
            advance();
            char esc = advance();
            switch (esc) {
                case 'n': lexeme += '\n'; break;
                case 't': lexeme += '\t'; break;
                case 'r': lexeme += '\r'; break;
                case '0': lexeme += '\0'; break;
                case '"': lexeme += '"'; break;
                case '\\': lexeme += '\\'; break;
                default: lexeme += esc; break;
            }
        } else {
            lexeme += advance();
        }
    }

    if (peek() == '\0') {
        return Token{TokenKind::Invalid, "Unterminated string literal", startLine, startColumn};
    }

    advance(); // consume closing quote '"'
    return Token{TokenKind::StringLiteral, lexeme, startLine, startColumn};
}

Token Lexer::charLiteral() {
    int startLine = line_;
    int startColumn = column_;
    advance(); // consume single quote '\''
    std::string lexeme;

    if (peek() == '\\') {
        advance();
        char esc = advance();
        switch (esc) {
            case 'n': lexeme += '\n'; break;
            case 't': lexeme += '\t'; break;
            case 'r': lexeme += '\r'; break;
            case '0': lexeme += '\0'; break;
            case '\'': lexeme += '\''; break;
            case '\\': lexeme += '\\'; break;
            default: lexeme += esc; break;
        }
    } else if (peek() != '\0' && peek() != '\'') {
        lexeme += advance();
    }

    if (peek() != '\'') {
        return Token{TokenKind::Invalid, "Unterminated character literal", startLine, startColumn};
    }
    advance(); // consume single quote '\''
    return Token{TokenKind::CharLiteral, lexeme, startLine, startColumn};
}

Token Lexer::identifier() {
    int startLine = line_;
    int startColumn = column_;
    std::string lexeme;
    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') {
        lexeme += advance();
    }

    static const std::unordered_map<std::string, TokenKind> keywords = {
        {"int", TokenKind::Int},         {"bool", TokenKind::Bool},
        {"char", TokenKind::Char},       {"float", TokenKind::Float},
        {"double", TokenKind::Double},   {"void", TokenKind::Void},
        {"true", TokenKind::True},       {"false", TokenKind::False},
        {"return", TokenKind::Return},   {"if", TokenKind::If},
        {"else", TokenKind::Else},       {"while", TokenKind::While},
        {"for", TokenKind::For},         {"do", TokenKind::Do},
        {"switch", TokenKind::Switch},   {"case", TokenKind::Case},
        {"default", TokenKind::Default}, {"break", TokenKind::Break},
        {"continue", TokenKind::Continue},{"struct", TokenKind::Struct},
        {"class", TokenKind::Class},     {"public", TokenKind::Public},
        {"private", TokenKind::Private}, {"protected", TokenKind::Protected},
        {"namespace", TokenKind::Namespace}, {"using", TokenKind::Using},
        {"const", TokenKind::Const},     {"static", TokenKind::Static},
        {"new", TokenKind::New},         {"delete", TokenKind::Delete}
    };

    auto it = keywords.find(lexeme);
    if (it != keywords.end()) {
        return Token{it->second, lexeme, startLine, startColumn};
    }
    return Token{TokenKind::Identifier, lexeme, startLine, startColumn};
}

Token Lexer::invalid(const std::string& message) {
    return Token{TokenKind::Invalid, message, line_, column_};
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        skipWhitespaceAndComments();
        int startLine = line_;
        int startColumn = column_;
        char c = peek();
        if (c == '\0') {
            tokens.push_back(Token{TokenKind::EndOfFile, "", startLine, startColumn});
            break;
        }

        if (std::isdigit(static_cast<unsigned char>(c))) {
            tokens.push_back(number());
            continue;
        }

        if (c == '"') {
            tokens.push_back(stringLiteral());
            continue;
        }

        if (c == '\'') {
            tokens.push_back(charLiteral());
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            tokens.push_back(identifier());
            continue;
        }

        advance();
        switch (c) {
            case '+':
                if (match('+')) {
                    tokens.push_back(makeToken(TokenKind::PlusPlus, "++"));
                } else if (match('=')) {
                    tokens.push_back(makeToken(TokenKind::PlusAssign, "+="));
                } else {
                    tokens.push_back(makeToken(TokenKind::Plus, "+"));
                }
                break;
            case '-':
                if (match('-')) {
                    tokens.push_back(makeToken(TokenKind::MinusMinus, "--"));
                } else if (match('=')) {
                    tokens.push_back(makeToken(TokenKind::MinusAssign, "-="));
                } else if (match('>')) {
                    tokens.push_back(makeToken(TokenKind::Arrow, "->"));
                } else {
                    tokens.push_back(makeToken(TokenKind::Minus, "-"));
                }
                break;
            case '*':
                if (match('=')) {
                    tokens.push_back(makeToken(TokenKind::StarAssign, "*="));
                } else {
                    tokens.push_back(makeToken(TokenKind::Star, "*"));
                }
                break;
            case '/':
                if (match('=')) {
                    tokens.push_back(makeToken(TokenKind::SlashAssign, "/="));
                } else {
                    tokens.push_back(makeToken(TokenKind::Slash, "/"));
                }
                break;
            case '%':
                if (match('=')) {
                    tokens.push_back(makeToken(TokenKind::PercentAssign, "%="));
                } else {
                    tokens.push_back(makeToken(TokenKind::Percent, "%"));
                }
                break;
            case '(': tokens.push_back(makeToken(TokenKind::LParen, "(")); break;
            case ')': tokens.push_back(makeToken(TokenKind::RParen, ")")); break;
            case '{': tokens.push_back(makeToken(TokenKind::LBrace, "{")); break;
            case '}': tokens.push_back(makeToken(TokenKind::RBrace, "}")); break;
            case '[': tokens.push_back(makeToken(TokenKind::LBracket, "[")); break;
            case ']': tokens.push_back(makeToken(TokenKind::RBracket, "]")); break;
            case ';': tokens.push_back(makeToken(TokenKind::Semicolon, ";")); break;
            case ',': tokens.push_back(makeToken(TokenKind::Comma, ",")); break;
            case '.': tokens.push_back(makeToken(TokenKind::Dot, ".")); break;
            case '?': tokens.push_back(makeToken(TokenKind::Question, "?")); break;
            case '~': tokens.push_back(makeToken(TokenKind::Tilde, "~")); break;
            case '^': tokens.push_back(makeToken(TokenKind::Caret, "^")); break;
            case ':':
                if (match(':')) {
                    tokens.push_back(makeToken(TokenKind::ColonColon, "::"));
                } else {
                    tokens.push_back(makeToken(TokenKind::Colon, ":"));
                }
                break;
            case '=':
                if (match('=')) {
                    tokens.push_back(makeToken(TokenKind::Eq, "=="));
                } else {
                    tokens.push_back(makeToken(TokenKind::Assign, "="));
                }
                break;
            case '!':
                if (match('=')) {
                    tokens.push_back(makeToken(TokenKind::Neq, "!="));
                } else {
                    tokens.push_back(makeToken(TokenKind::Bang, "!"));
                }
                break;
            case '<':
                if (match('=')) {
                    tokens.push_back(makeToken(TokenKind::Le, "<="));
                } else if (match('<')) {
                    tokens.push_back(makeToken(TokenKind::ShiftLeft, "<<"));
                } else {
                    tokens.push_back(makeToken(TokenKind::Lt, "<"));
                }
                break;
            case '>':
                if (match('=')) {
                    tokens.push_back(makeToken(TokenKind::Ge, ">="));
                } else if (match('>')) {
                    tokens.push_back(makeToken(TokenKind::ShiftRight, ">>"));
                } else {
                    tokens.push_back(makeToken(TokenKind::Gt, ">"));
                }
                break;
            case '&':
                if (match('&')) {
                    tokens.push_back(makeToken(TokenKind::AndAnd, "&&"));
                } else {
                    tokens.push_back(makeToken(TokenKind::Ampersand, "&"));
                }
                break;
            case '|':
                if (match('|')) {
                    tokens.push_back(makeToken(TokenKind::OrOr, "||"));
                } else {
                    tokens.push_back(makeToken(TokenKind::Pipe, "|"));
                }
                break;
            default:
                tokens.push_back(invalid(std::string("unexpected character '") + c + "'"));
                break;
        }
    }
    return tokens;
}
