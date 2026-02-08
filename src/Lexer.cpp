#include "include/Lexer.hpp"
#include "include/Token.hpp"
#include <cctype>
#include <iostream>
#include <ostream>
#include <unordered_map>

// 1. Define the Keywords Map
const std::unordered_map<std::string, TokenType> keywordMap = {
    {"int", TokenType::INT},          {"return", TokenType::RETURN},
    {"void", TokenType::VOID},        {"if", TokenType::IF},
    {"string", TokenType::STRING},    {"bool", TokenType::BOOL},
    {"true", TokenType::TRUE},        {"false", TokenType::FALSE},
    {"else", TokenType::ELSE},        {"while", TokenType::WHILE},
    {"for", TokenType::FOR},          {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE}};

Lexer::Lexer(const std::string &source) : m_src(source), m_pos(0) {};

char Lexer::peek(const int offset) const {
    if (m_pos + offset >= m_src.length())
        return '\0';
    return m_src[m_pos + offset];
}

char Lexer::consume() {
    if (m_pos >= m_src.length())
        return '\0';
    return m_src[m_pos++];
}

void Lexer::skipWhitespace() {
    while (std::isspace(peek())) {
        consume();
    }
}

bool Lexer::skipComments() {
    if (peek() != '/') {
        return false;
    }
    consume();
    // Single line comment
    if (peek() == '/') {
        while (peek() != '\n') {
            consume();
        }
        consume();
        return true;
    }
    // Multiline comment
    if (peek() == '*') {
        char prev = '/';
        while (peek() != '/' || prev != '*') {
            prev = consume();
        }
        consume();
        return true;
    }
    m_pos--;
    return false;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (peek() != '\0') {
        skipWhitespace();
        if (peek() == '\0')
            break;

        if (skipComments())
            continue;

        char current = peek();

        if (current == '"') {
            std::string buf;
            consume();
            while (peek() != '"' && peek() != '\0') {
                buf += consume();
            }
            consume();
            tokens.push_back({TokenType::STRING_LIT, buf});
            continue;
        }

        // --- Handle Identifiers and Keywords ---
        else if (isalpha(current)) {
            std::string buf;
            while (isalnum(peek())) {
                buf += consume();
            }

            if (keywordMap.count(buf)) {
                TokenType type = keywordMap.at(buf);
                if (type == TokenType::TRUE || type == TokenType::FALSE) {
                    tokens.push_back(
                        {type, std::to_string((int)(type == TokenType::TRUE))});
                } else {
                    tokens.push_back({type, buf});
                }
            } else {
                tokens.push_back({TokenType::IDENTIFIER, buf});
            }
        }
        // --- Handle Numbers ---
        else if (isdigit(current)) {
            std::string buf;
            while (isdigit(peek())) {
                buf += consume();
            }
            tokens.push_back({TokenType::INT_LIT, buf});
        }
        // --- Handle Symbols & Operators ---
        else {
            consume();
            switch (current) {
            // Single-char tokens
            case ';':
                tokens.push_back({TokenType::SEMI, ";"});
                break;
            case '(':
                tokens.push_back({TokenType::LPAREN, "("});
                break;
            case ')':
                tokens.push_back({TokenType::RPAREN, ")"});
                break;
            case '{':
                tokens.push_back({TokenType::LBRACE, "{"});
                break;
            case '}':
                tokens.push_back({TokenType::RBRACE, "}"});
                break;
            case ',':
                tokens.push_back({TokenType::COMMA, ","});
                break;
            case '+':
                if (peek() == '=') {
                    consume();
                    Token var = tokens[tokens.size() - 1];
                    tokens.push_back({TokenType::ASSIGN, "="});
                    tokens.push_back(var);
                } else if (peek() == '+') {
                    consume();
                    Token var = tokens[tokens.size() - 1];
                    tokens.push_back({TokenType::ASSIGN, "="});
                    tokens.push_back(var);
                    tokens.push_back({TokenType::PLUS, "+"});
                    tokens.push_back({TokenType::INT_LIT, "1"});
                    break;
                }
                tokens.push_back({TokenType::PLUS, "+"});
                break;
            case '-':
                if (peek() == '=') {
                    consume();
                    Token var = tokens[tokens.size() - 1];
                    tokens.push_back({TokenType::ASSIGN, "="});
                    tokens.push_back(var);
                } else if (peek() == '-') {
                    consume();
                    Token var = tokens[tokens.size() - 1];
                    tokens.push_back({TokenType::ASSIGN, "="});
                    tokens.push_back(var);
                    tokens.push_back({TokenType::MINUS, "-"});
                    tokens.push_back({TokenType::INT_LIT, "1"});
                    break;
                }
                tokens.push_back({TokenType::MINUS, "-"});
                break;
            case '*':
                if (peek() == '=') {
                    consume();
                    Token var = tokens[tokens.size() - 1];
                    tokens.push_back({TokenType::ASSIGN, "="});
                    tokens.push_back(var);
                }
                tokens.push_back({TokenType::STAR, "*"});
                break;
            case '/':
                if (peek() == '=') {
                    consume();
                    Token var = tokens[tokens.size() - 1];
                    tokens.push_back({TokenType::ASSIGN, "="});
                    tokens.push_back(var);
                }
                tokens.push_back({TokenType::SLASH, "/"});
                break;

            // Multi-char tokens (==, !=, <=, >=)
            case '=':
                if (peek() == '=') {
                    consume(); // Eat the second '='
                    tokens.push_back({TokenType::EQ, "=="});
                } else {
                    tokens.push_back({TokenType::ASSIGN, "="});
                }
                break;

            case '!':
                if (peek() == '=') {
                    consume();
                    tokens.push_back({TokenType::NEQ, "!="});
                } else {
                    tokens.push_back({TokenType::NOT, "!"});
                }
                break;

            case '<':
                if (peek() == '=') {
                    consume();
                    tokens.push_back({TokenType::LE, "<="});
                } else {
                    tokens.push_back({TokenType::LT, "<"});
                }
                break;

            case '>':
                if (peek() == '=') {
                    consume();
                    tokens.push_back({TokenType::GE, ">="});
                } else {
                    tokens.push_back({TokenType::GT, ">"});
                }
                break;

            default:
                std::cerr << "LEXER: Unknown character: '" << current << "'"
                          << std::endl;
                break;
            }
        }
    }

    tokens.push_back({TokenType::TOK_EOF, ""});
    return tokens;
}
