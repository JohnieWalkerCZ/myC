#include "include/Lexer.hpp"
#include "include/Token.hpp"
#include <cctype>
#include <iostream>
#include <ostream>
#include <stdexcept>
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

Lexer::Lexer(const std::string &source)
    : m_src(source), m_pos(0), m_col(0), m_row(1) {};

char Lexer::peek(const int offset) const {
    if (m_pos + offset >= m_src.length())
        return '\0';
    return m_src[m_pos + offset];
}

char Lexer::consume() {
    if (m_pos >= m_src.length())
        return '\0';
    m_col++;
    if (peek() == '\n') {
        m_row++;
        m_col = 0;
    }
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
            tokens.push_back(
                {TokenType::STRING_LIT, buf, m_col - (int)buf.length() + 1, m_row});
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
                        {type, std::to_string((int)(type == TokenType::TRUE)),
                         m_col - (int)buf.length() + 1, m_row});
                } else {
                    tokens.push_back(
                        {type, buf, m_col - (int)buf.length() + 1, m_row});
                }
            } else {
                tokens.push_back({TokenType::IDENTIFIER, buf,
                                  m_col - (int)buf.length() + 1, m_row});
            }
        }
        // --- Handle Numbers ---
        else if (isdigit(current)) {
            std::string buf;
            while (isdigit(peek())) {
                buf += consume();
            }
            tokens.push_back(
                {TokenType::INT_LIT, buf, m_col - (int)buf.length() + 1, m_row});
        }
        // --- Handle Symbols & Operators ---
        else {
            consume();
            switch (current) {
            // Single-char tokens
            case ';':
                tokens.push_back({TokenType::SEMI, ";", m_col, m_row});
                break;
            case '(':
                tokens.push_back({TokenType::LPAREN, "(", m_col, m_row});
                break;
            case ')':
                tokens.push_back({TokenType::RPAREN, ")", m_col, m_row});
                break;
            case '{':
                tokens.push_back({TokenType::LBRACE, "{", m_col, m_row});
                break;
            case '}':
                tokens.push_back({TokenType::RBRACE, "}", m_col, m_row});
                break;
            case '[':
                tokens.push_back({TokenType::LBRACKET, "[", m_col, m_row});
                break;
            case ']':
                tokens.push_back({TokenType::RBRACKET, "]", m_col, m_row});
                break;
            case ',':
                tokens.push_back({TokenType::COMMA, ",", m_col, m_row});
                break;
            case '+':
                if (peek() == '=') {
                    consume();
                    Token var = tokens[tokens.size() - 1];
                    tokens.push_back({TokenType::ASSIGN, "=", m_col, m_row});
                    tokens.push_back(var);
                } else if (peek() == '+') {
                    consume();
                    Token var = tokens[tokens.size() - 1];
                    tokens.push_back({TokenType::ASSIGN, "=", m_col, m_row});
                    tokens.push_back(var);
                    tokens.push_back({TokenType::PLUS, "+", m_col, m_row});
                    tokens.push_back({TokenType::INT_LIT, "1", m_col, m_row});
                    break;
                }
                tokens.push_back({TokenType::PLUS, "+", m_col, m_row});
                break;
            case '-':
                if (peek() == '=') {
                    consume();
                    Token var = tokens[tokens.size() - 1];
                    tokens.push_back({TokenType::ASSIGN, "=", m_col, m_row});
                    tokens.push_back(var);
                } else if (peek() == '-') {
                    consume();
                    Token var = tokens[tokens.size() - 1];
                    tokens.push_back({TokenType::ASSIGN, "=", m_col, m_row});
                    tokens.push_back(var);
                    tokens.push_back({TokenType::MINUS, "-", m_col, m_row});
                    tokens.push_back({TokenType::INT_LIT, "1", m_col, m_row});
                    break;
                }
                tokens.push_back({TokenType::MINUS, "-", m_col, m_row});
                break;
            case '*':
                if (peek() == '=') {
                    consume();
                    Token var = tokens[tokens.size() - 1];
                    tokens.push_back({TokenType::ASSIGN, "=", m_col, m_row});
                    tokens.push_back(var);
                }
                tokens.push_back({TokenType::STAR, "*", m_col, m_row});
                break;
            case '/':
                if (peek() == '=') {
                    consume();
                    Token var = tokens[tokens.size() - 1];
                    tokens.push_back({TokenType::ASSIGN, "=", m_col, m_row});
                    tokens.push_back(var);
                }
                tokens.push_back({TokenType::SLASH, "/", m_col, m_row});
                break;

            // Multi-char tokens (==, !=, <=, >=)
            case '=':
                if (peek() == '=') {
                    consume(); // Eat the second '='
                    tokens.push_back({TokenType::EQ, "==", m_col, m_row});
                } else {
                    tokens.push_back({TokenType::ASSIGN, "=", m_col, m_row});
                }
                break;

            case '!':
                if (peek() == '=') {
                    consume();
                    tokens.push_back({TokenType::NEQ, "!=", m_col, m_row});
                } else {
                    tokens.push_back({TokenType::NOT, "!", m_col, m_row});
                }
                break;

            case '<':
                if (peek() == '=') {
                    consume();
                    tokens.push_back({TokenType::LE, "<=", m_col, m_row});
                } else {
                    tokens.push_back({TokenType::LT, "<", m_col, m_row});
                }
                break;

            case '>':
                if (peek() == '=') {
                    consume();
                    tokens.push_back({TokenType::GE, ">=", m_col, m_row});
                } else {
                    tokens.push_back({TokenType::GT, ">", m_col, m_row});
                }
                break;
            case '%':
                if (peek() == '=') {
                    consume();
                    Token var = tokens[tokens.size() - 1];
                    tokens.push_back({TokenType::ASSIGN, "=", m_col, m_row});
                    tokens.push_back(var);
                }
                tokens.push_back({TokenType::MOD, "%", m_col, m_row});
                break;

            default:
                throw std::runtime_error(
                    "Lexer: Unknown characker '" + std::string(1, current) +
                    "' at position: " + std::to_string(m_row) + ":" +
                    std::to_string(m_col));
                break;
            }
        }
    }

    tokens.push_back({TokenType::TOK_EOF, ""});
    return tokens;
}
