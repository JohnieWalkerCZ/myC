#pragma once
#include "AST.hpp"
#include <map>
#include <string>

// Simple enum to track types during checking
enum class ExprType { INT, BOOL, VOID, STRING, UNKNOWN };

class TypeChecker {
  public:
    void checkProgram(const ProgramNode *program);

  private:
    // We need a separate symbol table for types (Name -> Type)
    std::map<std::string, ExprType> m_symbolTable;

    // Tracks the current function's return type (for return checking)
    ExprType m_currentFuncReturnType = ExprType::UNKNOWN;

    void checkFunction(const FunctionDeclNode *func);
    void checkBlock(const BlockNode *block);
    void checkStatement(const ASTNode *stmt);

    // Determines the type of an expression node
    ExprType inferType(const ASTNode *node);

    // Helpers
    ExprType tokenToType(const std::string &typeName);
    std::string typeToString(ExprType type);
};
