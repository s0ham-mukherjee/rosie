#include "token.hpp"

const char* tokenKindName(TokenKind kind) {
    switch (kind) {
        case TokenKind::Int: return "int";
        case TokenKind::Bool: return "bool";
        case TokenKind::Char: return "char";
        case TokenKind::Float: return "float";
        case TokenKind::Double: return "double";
        case TokenKind::Void: return "void";
        case TokenKind::True: return "true";
        case TokenKind::False: return "false";
        case TokenKind::Return: return "return";
        case TokenKind::If: return "if";
        case TokenKind::Else: return "else";
        case TokenKind::While: return "while";
        case TokenKind::For: return "for";
        case TokenKind::Do: return "do";
        case TokenKind::Switch: return "switch";
        case TokenKind::Case: return "case";
        case TokenKind::Default: return "default";
        case TokenKind::Break: return "break";
        case TokenKind::Continue: return "continue";
        case TokenKind::Struct: return "struct";
        case TokenKind::Class: return "class";
        case TokenKind::Public: return "public";
        case TokenKind::Private: return "private";
        case TokenKind::Protected: return "protected";
        case TokenKind::Namespace: return "namespace";
        case TokenKind::Using: return "using";
        case TokenKind::Const: return "const";
        case TokenKind::Static: return "static";
        case TokenKind::New: return "new";
        case TokenKind::Delete: return "delete";

        case TokenKind::Number: return "number";
        case TokenKind::FloatLiteral: return "float_literal";
        case TokenKind::CharLiteral: return "char_literal";
        case TokenKind::StringLiteral: return "string_literal";
        case TokenKind::Identifier: return "identifier";

        case TokenKind::Plus: return "+";
        case TokenKind::Minus: return "-";
        case TokenKind::Star: return "*";
        case TokenKind::Slash: return "/";
        case TokenKind::Percent: return "%";
        case TokenKind::PlusPlus: return "++";
        case TokenKind::MinusMinus: return "--";
        case TokenKind::Assign: return "=";
        case TokenKind::PlusAssign: return "+=";
        case TokenKind::MinusAssign: return "-=";
        case TokenKind::StarAssign: return "*=";
        case TokenKind::SlashAssign: return "/=";
        case TokenKind::PercentAssign: return "%=";
        case TokenKind::Eq: return "==";
        case TokenKind::Neq: return "!=";
        case TokenKind::Lt: return "<";
        case TokenKind::Gt: return ">";
        case TokenKind::Le: return "<=";
        case TokenKind::Ge: return ">=";
        case TokenKind::AndAnd: return "&&";
        case TokenKind::OrOr: return "||";
        case TokenKind::Bang: return "!";
        case TokenKind::Ampersand: return "&";
        case TokenKind::Pipe: return "|";
        case TokenKind::Caret: return "^";
        case TokenKind::Tilde: return "~";
        case TokenKind::ShiftLeft: return "<<";
        case TokenKind::ShiftRight: return ">>";
        case TokenKind::Question: return "?";
        case TokenKind::Colon: return ":";
        case TokenKind::ColonColon: return "::";
        case TokenKind::Dot: return ".";
        case TokenKind::Arrow: return "->";

        case TokenKind::LParen: return "(";
        case TokenKind::RParen: return ")";
        case TokenKind::LBrace: return "{";
        case TokenKind::RBrace: return "}";
        case TokenKind::LBracket: return "[";
        case TokenKind::RBracket: return "]";
        case TokenKind::Semicolon: return ";";
        case TokenKind::Comma: return ",";
        case TokenKind::EndOfFile: return "EOF";
        case TokenKind::Invalid: return "invalid";
    }
    return "?";
}
