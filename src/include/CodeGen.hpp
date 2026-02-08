#pragma once

#include "AST.hpp"
#include "Token.hpp"
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

struct VarInfo {
    int offset;
    std::string type;
};

class CodeGen {
  public:
    CodeGen(const std::string &filename);
    ~CodeGen();

    void generateProgram(const ProgramNode *program);

  private:
    std::ofstream m_out;
    std::unordered_map<std::string, VarInfo> m_vars;
    std::unordered_map<std::string, TokenType> m_var_types;
    int m_stackIndex;
    int m_labelCounter;

    void push(const std::string &reg);
    void pop(const std::string &reg);
    void emit(const std::string &fmt);
    void emitLabel(const std::string &label);
    std::string getUniqueLabel(const std::string &prefix = "L");

    void generate(const ASTNode *node);
    void generateBlock(const BlockNode *node);
    void generateFunction(const FunctionDeclNode *node);
    void genVarAssign(const VarAssignNode *node);

    void genBinaryOp(const BinaryOpNode *node);
    void genUnaryOp(const UnaryOpNode *node);
    void genVarDecl(const VarDeclNode *node);
    void genIf(const IfNode *node);
    void genWhile(const WhileNode *node);
    void genFunctionCall(const FunctionCallNode *node);

    void genPrintCall(const FunctionCallNode *node);

    struct DataEntry {
        std::string label;
        std::string value;
    };
    std::vector<DataEntry> m_rodata;
    std::string makeStringLiteral(const std::string &value);
};
