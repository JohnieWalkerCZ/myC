#pragma once

#include "AST.hpp"
#include "Token.hpp"
#include <cstddef>
#include <memory>
#include <vector>

using Node = std::unique_ptr<ASTNode>;

class Parser {
  public:
    Parser(const std::vector<Token> &tokens);

    std::unique_ptr<ProgramNode> parseProgram();

  private:
    std::vector<Token> m_tokens;
    size_t m_pos;

    Token peek(int offset = 0) const;
    Token advance();
    bool match(TokenType type);
    Token consume(TokenType type, const std::string &errMsg);

    std::unique_ptr<FunctionDeclNode> parseFunctionDeclaration();

    std::unique_ptr<BlockNode> parseBlock();

    Node parseStatement();

    std::unique_ptr<VarDeclNode> parseVarDeclaration();

    std::unique_ptr<ReturnNode> parseReturnStatement();

    std::unique_ptr<IfNode> parseIfStatement();

    std::unique_ptr<WhileNode> parseWhileStatement();

    std::unique_ptr<ForNode> parseForStatement();

    Node parseExpression();

    Node parseAssignment();
    Node parseUnary();
    Node parseLogicalOr();
    Node parseLogicalAnd();
    Node parseEquality();
    Node parseRelational();
    Node parseAdditive();
    Node parseMultiplicative();
    Node parsePrimary();
};
