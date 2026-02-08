#include "AST.hpp"
#include "Token.hpp"
#include <string>
#include <vector>

std::string tokenTypeToString(TokenType type);

void printTokens(const std::vector<Token> &tokens);

void printIndent(int indent);

void printAST(const ASTNode *node, int indent = 0);
