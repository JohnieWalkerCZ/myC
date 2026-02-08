#pragma once
#include <string>

enum class TokenType {
    INT,
    VOID,
    STRING,
    BOOL,

    TRUE,
    FALSE,

    RETURN,
    IF,
    ELSE,
    WHILE,
    FOR,
    BREAK,
    CONTINUE,

    IDENTIFIER,
    INT_LIT,
    STRING_LIT,

    PLUS,
    MINUS,
    STAR,
    SLASH,
    MOD,
    ASSIGN,
    EQ,
    NEQ,
    LT,
    LE,
    GT,
    GE,
    AND,
    OR,
    NOT,

    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    SEMI,
    COMMA,

    TOK_EOF
};

struct Token {
    TokenType type;
    std::string value;
};

inline bool isDataType(TokenType t) {
    return t == TokenType::INT || t == TokenType::VOID ||
           t == TokenType::BOOL || t == TokenType::STRING;
}
