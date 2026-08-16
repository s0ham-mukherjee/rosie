#pragma once

#include <string>

enum class TokenKind {
    // Keywords & Types
    Int,
    Bool,
    Char,
    Float,
    Double,
    Void,
    True,
    False,
    Return,
    If,
    Else,
    While,
    For,
    Do,
    Switch,
    Case,
    Default,
    Break,
    Continue,
    Struct,
    Class,
    Public,
    Private,
    Protected,
    Namespace,
    Using,
    Const,
    Static,
    New,
    Delete,

    // Literals & Identifiers
    Number,
    FloatLiteral,
    CharLiteral,
    StringLiteral,
    Identifier,

    // Operators
    Plus,          // +
    Minus,         // -
    Star,          // *
    Slash,         // /
    Percent,       // %
    PlusPlus,      // ++
    MinusMinus,    // --
    Assign,        // =
    PlusAssign,    // +=
    MinusAssign,   // -=
    StarAssign,    // *=
    SlashAssign,   // /=
    PercentAssign, // %=
    Eq,            // ==
    Neq,           // !=
    Lt,            // <
    Gt,            // >
    Le,            // <=
    Ge,            // >=
    AndAnd,        // &&
    OrOr,          // ||
    Bang,          // !
    Ampersand,     // &
    Pipe,          // |
    Caret,         // ^
    Tilde,         // ~
    ShiftLeft,     // <<
    ShiftRight,    // >>
    Question,      // ?
    Colon,         // :
    ColonColon,    // ::
    Dot,           // .
    Arrow,         // ->

    // Delimiters
    LParen,        // (
    RParen,        // )
    LBrace,        // {
    RBrace,        // }
    LBracket,      // [
    RBracket,      // ]
    Semicolon,     // ;
    Comma,         // ,
    EndOfFile,
    Invalid
};

struct Token {
    TokenKind kind = TokenKind::Invalid;
    std::string lexeme;
    int line = 1;
    int column = 1;
};

const char* tokenKindName(TokenKind kind);
