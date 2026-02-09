#include "include/Parser.hpp"
#include "include/AST.hpp"
#include "include/Debug.hpp"
#include "include/Token.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using Node = std::unique_ptr<ASTNode>;

Parser::Parser(const std::vector<Token> &tokens)
    : m_tokens(tokens), m_pos(0) {};

Token Parser::peek(int offset) const {
    if (m_pos + offset >= m_tokens.size())
        return m_tokens.back();

    return m_tokens[m_pos + offset];
}

Token Parser::advance() {
    if (m_pos < m_tokens.size()) {
        m_pos++;
    }

    return m_tokens[m_pos - 1];
}

bool Parser::match(TokenType type) {
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string &errMsg) {
    if (peek().type == type) {
        return advance();
    }

    throw std::runtime_error("PARSER: " + errMsg + " at '" + peek().value +
                             "'");
}

std::unique_ptr<ProgramNode> Parser::parseProgram() {
    auto program = std::make_unique<ProgramNode>();

    while (peek().type != TokenType::TOK_EOF) {
        program->functions.push_back(parseFunctionDeclaration());
    }
    return program;
}

std::unique_ptr<FunctionDeclNode> Parser::parseFunctionDeclaration() {
    if (peek().type != TokenType::INT && peek().type != TokenType::VOID &&
        peek().type != TokenType::STRING && peek().type != TokenType::BOOL) {
        throw std::runtime_error(
            "PARSER: Expected return type for function definition.");
    }

    std::string type = advance().value;

    std::string name =
        consume(TokenType::IDENTIFIER, "Expected function name").value;

    consume(TokenType::LPAREN, "Expected '(' after function name");

    std::vector<FunctionDeclNode::Param> params;
    if (peek().type != TokenType::RPAREN) {
        do {
            std::string pType = advance().value;
            std::string pName =
                consume(TokenType::IDENTIFIER, "Expected parameter name").value;
            params.push_back({pType, pName});
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RPAREN, "Expected ')' after parameters");

    consume(TokenType::LBRACE, "Expected '{' before functions body");
    auto body = parseBlock();

    return std::make_unique<FunctionDeclNode>(type, name, params,
                                              std::move(body));
}

std::unique_ptr<BlockNode> Parser::parseBlock() {
    auto block = std::make_unique<BlockNode>();

    while (peek().type != TokenType::RBRACE) {
        block->statements.push_back(parseStatement());
    }

    consume(TokenType::RBRACE, "Expected '}' after block");
    return block;
}

Node Parser::parseStatement() {
    if (peek().type == TokenType::INT || peek().type == TokenType::BOOL ||
        peek().type == TokenType::STRING) {
        return parseVarDeclaration();
    }
    if (peek().type == TokenType::RETURN) {
        return parseReturnStatement();
    }
    if (peek().type == TokenType::IF) {
        return parseIfStatement();
    }
    if (peek().type == TokenType::WHILE) {
        return parseWhileStatement();
    }
    if (peek().type == TokenType::FOR) {
        return parseForStatement();
    }
    if (peek().type == TokenType::LBRACE) {
        consume(TokenType::LBRACE, "Expected '{'");
        return parseBlock();
    }

    auto expr = parseExpression();
    consume(TokenType::SEMI, "Expected ';' after expression");
    return expr;
}

Node Parser::parseVarDeclaration() {
    std::string type = advance().value;
    std::string name =
        consume(TokenType::IDENTIFIER, "Expected variable name").value;
    if (peek().type == TokenType::LBRACKET) {
        return parseArrayVarDeclaration(type, name);
    }
    return parseUnitVarDeclaration(type, name);
}

std::unique_ptr<VarDeclNode> Parser::parseUnitVarDeclaration(std::string type,
                                                             std::string name) {
    Node initExpr = nullptr;
    if (match(TokenType::ASSIGN)) {
        initExpr = parseExpression();
    }

    consume(TokenType::SEMI, "Expected ';' after variable declaration");
    return std::make_unique<VarDeclNode>(type, name, std::move(initExpr));
}

std::unique_ptr<ArrayDeclNode>
Parser::parseArrayVarDeclaration(std::string type, std::string name) {
    consume(TokenType::LBRACKET, "Expected '['");

    Token sizeToken =
        consume(TokenType::INT_LIT, "Array size must be a constant integer");
    int sizeVal = std::stoi(sizeToken.value);

    consume(TokenType::RBRACKET, "Expected ']'");
    consume(TokenType::SEMI, "Expected ';' after array declaration");

    return std::make_unique<ArrayDeclNode>(type, name, sizeVal);
}

std::unique_ptr<ReturnNode> Parser::parseReturnStatement() {
    consume(TokenType::RETURN, "Expected 'return'");

    Node value = nullptr;
    if (peek().type != TokenType::SEMI) {
        value = parseExpression();
    }

    consume(TokenType::SEMI, "Expected ';' after return");
    return std::make_unique<ReturnNode>(std::move(value));
}
std::unique_ptr<IfNode> Parser::parseIfStatement() {
    advance();
    consume(TokenType::LPAREN, "Expected '(' after if");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "Exprected ')' after condition");

    auto thenBranch = parseStatement();
    std::unique_ptr<ASTNode> elseBranch = nullptr;

    if (match(TokenType::ELSE)) {
        if (peek().type == TokenType::IF) {
            elseBranch = parseIfStatement();
        } else {
            elseBranch = parseStatement();
        }
    }

    return std::make_unique<IfNode>(std::move(condition), std::move(thenBranch),
                                    std::move(elseBranch));
}

std::unique_ptr<WhileNode> Parser::parseWhileStatement() {
    advance();
    consume(TokenType::LPAREN, "Expected '(' after while");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after condition");

    auto body = parseStatement();

    return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

std::unique_ptr<ForNode> Parser::parseForStatement() {
    advance();
    consume(TokenType::LPAREN, "Expected '(' after for");

    Node init = nullptr;
    if (peek().type != TokenType::SEMI) {
        if (peek().type == TokenType::INT || peek().type == TokenType::BOOL ||
            peek().type == TokenType::STRING) {
            init = parseVarDeclaration();
        } else {
            init = parseExpression();
            consume(TokenType::SEMI,
                    "Expected ';' after initialization expression");
        }
    } else {
        advance();
    }

    Node condition = nullptr;
    if (peek().type != TokenType::SEMI) {
        condition = parseExpression();
    }
    consume(TokenType::SEMI, "Expected ';' after condition");

    Node update = nullptr;
    if (peek().type != TokenType::RPAREN) {
        update = parseExpression();
    }
    consume(TokenType::RPAREN, "Expected ')' after update");

    auto body = parseStatement();

    return std::make_unique<ForNode>(std::move(init), std::move(condition),
                                     std::move(update), std::move(body));
}
Node Parser::parseExpression() { return parseAssignment(); }

// Root of waterfall -> dictates order of operation (bottom up)
/*
 * =
 * UNARY (+, -, !)
 * ||
 * &&
 * == / !=
 * < / <= / > / >=
 * + / -
 * * / / / %
 * 123 / variables / parentheses / function calls
 */

Node Parser::parseAssignment() {
    auto left = parseUnary();
    if (match(TokenType::ASSIGN)) {
        auto right = parseAssignment();
        if (auto *varNode = dynamic_cast<VariableNode *>(left.get())) {
            return std::make_unique<VarAssignNode>(varNode->name,
                                                   std::move(right));
        }
        if (auto *arrayNode = dynamic_cast<ArrayAccessNode *>(left.get())) {
            return std::make_unique<ArrayAssignNode>(
                arrayNode->name, std::move(arrayNode->index), std::move(right));
        }
    }
    return left;
}

Node Parser::parseUnary() {
    if (peek().type == TokenType::NOT || peek().type == TokenType::MINUS ||
        peek().type == TokenType::PLUS) {
        auto op = advance().value;
        auto value = parseLogicalOr();
        return std::make_unique<UnaryOpNode>(op, std::move(value));
    }
    auto left = parseLogicalOr();
    return left;
}

Node Parser::parseLogicalOr() {
    auto left = parseLogicalAnd();
    while (match(TokenType::OR)) {
        auto right = parseLogicalAnd();
        left = std::make_unique<BinaryOpNode>("||", std::move(left),
                                              std::move(right));
    }
    return left;
}

Node Parser::parseLogicalAnd() {
    auto left = parseEquality();
    while (match(TokenType::AND)) {
        auto right = parseEquality();
        left = std::make_unique<BinaryOpNode>("&&", std::move(left),
                                              std::move(right));
    }
    return left;
}

Node Parser::parseEquality() {
    auto left = parseRelational();
    while (peek().type == TokenType::EQ || peek().type == TokenType::NEQ) {
        auto op = advance().value;
        auto right = parseRelational();
        left = std::make_unique<BinaryOpNode>(op, std::move(left),
                                              std::move(right));
    }
    return left;
}

Node Parser::parseRelational() {
    auto left = parseAdditive();
    while (peek().type == TokenType::LE || peek().type == TokenType::LT ||
           peek().type == TokenType::GE || peek().type == TokenType::GT) {
        auto op = advance().value;
        auto right = parseAdditive();
        left = std::make_unique<BinaryOpNode>(op, std::move(left),
                                              std::move(right));
    }
    return left;
}

Node Parser::parseAdditive() {
    auto left = parseMultiplicative();
    while (peek().type == TokenType::PLUS || peek().type == TokenType::MINUS) {
        auto op = advance().value;
        auto right = parseMultiplicative();
        left = std::make_unique<BinaryOpNode>(op, std::move(left),
                                              std::move(right));
    }
    return left;
}

Node Parser::parseMultiplicative() {
    auto left = parsePrimary();
    while (peek().type == TokenType::STAR || peek().type == TokenType::SLASH ||
           peek().type == TokenType::MOD) {
        auto op = advance().value;
        auto right = parsePrimary();
        left = std::make_unique<BinaryOpNode>(op, std::move(left),
                                              std::move(right));
    }
    return left;
}

Node Parser::parsePrimary() {
    if (match(TokenType::INT_LIT)) {
        return std::make_unique<NumberNode>(
            std::stoi(m_tokens[m_pos - 1].value));
    }
    if (match(TokenType::STRING_LIT)) {
        std::string val = m_tokens[m_pos - 1].value;
        return std::make_unique<StringNode>(val);
    }

    if (match(TokenType::TRUE) || match(TokenType::FALSE)) {
        return std::make_unique<BoolNode>(std::stoi(m_tokens[m_pos - 1].value));
    }

    if (match(TokenType::IDENTIFIER)) {
        std::string name = m_tokens[m_pos - 1].value;

        // Function calls identifier(...)
        if (match(TokenType::LPAREN)) {
            std::vector<Node> args;
            if (peek().type != TokenType::RPAREN) {
                args.push_back(parseExpression());
                while (match(TokenType::COMMA)) {
                    args.push_back(parseExpression());
                }
            }

            consume(TokenType::RPAREN, "Expected ')' after arguments");
            return std::make_unique<FunctionCallNode>(name, std::move(args));
        }

        if (match(TokenType::LBRACKET)) {
            Node index = parseExpression();
            consume(TokenType::RBRACKET, "Expected ']' after array access");
            return std::make_unique<ArrayAccessNode>(name, std::move(index));
        }

        return std::make_unique<VariableNode>(name);
    }

    if (match(TokenType::LPAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }

    if (match(TokenType::BREAK)) {
        return std::make_unique<BreakNode>();
    }

    if (match(TokenType::CONTINUE)) {
        return std::make_unique<ContinueNode>();
    }

    throw std::runtime_error(
        "PARSER (primary): Unexpected token in expression: " + peek().value);
}
