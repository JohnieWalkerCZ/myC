#include "include/TypeChecker.hpp"
#include "include/AST.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <ostream>

void TypeChecker::checkProgram(const ProgramNode *program) {
    for (const auto &func : program->functions) {
        FunctionSignature sig;
        sig.returnType = tokenToType(func->returnType);
        for (const auto &param : func->parameters) {
            sig.paramTypes.push_back(tokenToType(param.type));
        }
        m_functionTable[func->name] = sig;
    }

    for (const auto &func : program->functions) {
        checkFunction(func.get());
    }
}

void TypeChecker::checkFunction(const FunctionDeclNode *func) {
    m_symbolTable.clear();
    m_arrayTable.clear();

    for (const auto &param : func->parameters) {
        m_symbolTable[param.name] = tokenToType(param.type);
    }

    m_currentFuncReturnType = tokenToType(func->returnType);
    checkBlock(func->body.get());
}

void TypeChecker::checkBlock(const BlockNode *block) {
    for (const auto &stmt : block->statements) {
        checkStatement(stmt.get());
    }
}

void TypeChecker::checkStatement(const ASTNode *stmt) {
    if (auto node = dynamic_cast<const VarDeclNode *>(stmt)) {
        ExprType declaredType = tokenToType(node->type);

        if (node->initExpr) {
            ExprType exprType = inferType(node->initExpr.get());

            if (declaredType != exprType) {
                throw std::runtime_error(
                    "Type Mismatch: Cannot assign " + typeToString(exprType) +
                    " to variable '" + node->name + "' of type " +
                    typeToString(declaredType));
            }
        }

        m_symbolTable[node->name] = declaredType;
    } else if (auto node = dynamic_cast<const VarAssignNode *>(stmt)) {
        if (m_symbolTable.find(node->name) == m_symbolTable.end()) {
            throw std::runtime_error("Undefined variable: " + node->name);
        }

        ExprType varType = m_symbolTable[node->name];
        ExprType exprType = inferType(node->newExpr.get());

        if (varType != exprType) {
            throw std::runtime_error("Type Mismatch: Cannot assign " +
                                     typeToString(exprType) + " to variable '" +
                                     node->name + "'");
        }

    } else if (auto node = dynamic_cast<const ArrayDeclNode *>(stmt)) {
        ExprType declaredType = tokenToType(node->type);

        if (node->size <= 0) {
            throw std::runtime_error("Array size must be a positive integer.");
        }

        m_symbolTable[node->name] = declaredType;
        m_arrayTable[node->name] = node->size;

    } else if (auto node = dynamic_cast<const ArrayAssignNode *>(stmt)) {
        if (m_symbolTable.find(node->name) == m_symbolTable.end()) {
            throw std::runtime_error("Undefined array: " + node->name);
        }

        if (auto indexNode =
                dynamic_cast<const NumberNode *>(node->index.get())) {
            int index = indexNode->value;
            int size = m_arrayTable[node->name];

            if (index < 0 || index >= size) {
                throw std::runtime_error(
                    "Array Index Out of Bounds: Index " +
                    std::to_string(index) + " is out of range for array '" +
                    node->name + "' of size " + std::to_string(size));
            }
        }

        ExprType varType = m_symbolTable[node->name];
        ExprType exprType = inferType(node->value.get());

        if (varType != exprType) {
            throw std::runtime_error("Type mismatch: Cannot assign " +
                                     typeToString(exprType) + " to array '" +
                                     node->name + "'");
        }

    } else if (auto node = dynamic_cast<const ReturnNode *>(stmt)) {
        ExprType returnType = node->returnValue
                                  ? inferType(node->returnValue.get())
                                  : ExprType::VOID;
        if (returnType != m_currentFuncReturnType) {
            throw std::runtime_error("Return type mismatch!");
        }
    } else if (auto node = dynamic_cast<const IfNode *>(stmt)) {
        ExprType condType = inferType(node->condition.get());
        if (condType != ExprType::BOOL) {
            throw std::runtime_error("If statement condition must be BOOL");
        }
        checkStatement(node->thenBranch.get());
        if (node->elseBranch)
            checkStatement(node->elseBranch.get());
    } else if (auto node = dynamic_cast<const WhileNode *>(stmt)) {
        ExprType condType = inferType(node->condition.get());
        if (condType != ExprType::BOOL) {
            throw std::runtime_error("While statement condition must be BOOL");
        }
        checkStatement(node->body.get());
    }
}

ExprType TypeChecker::inferType(const ASTNode *node) {
    if (dynamic_cast<const NumberNode *>(node))
        return ExprType::INT;
    if (dynamic_cast<const BoolNode *>(node))
        return ExprType::BOOL;
    if (dynamic_cast<const StringNode *>(node))
        return ExprType::STRING;

    if (auto var = dynamic_cast<const VariableNode *>(node)) {
        if (m_symbolTable.find(var->name) == m_symbolTable.end()) {
            throw std::runtime_error("Undefined variable: " + var->name);
        }
        return m_symbolTable[var->name];
    }

    if (auto binOp = dynamic_cast<const BinaryOpNode *>(node)) {
        ExprType left = inferType(binOp->left.get());
        ExprType right = inferType(binOp->right.get());

        // Arithmetic (+, -, *, /) results in INT
        if (binOp->op == "+" || binOp->op == "-" || binOp->op == "*" ||
            binOp->op == "/" || binOp->op == "%") {
            if (left != ExprType::INT || right != ExprType::INT) {
                throw std::runtime_error(
                    "Arithmetic operations require INT operands");
            }
            return ExprType::INT;
        }

        // Relational (<, >, ==) results in BOOL
        if (binOp->op == "==" || binOp->op == "<" || binOp->op == ">" ||
            binOp->op == ">=" || binOp->op == "<=") {
            if (left != right) {
                throw std::runtime_error("Cannot compare different types");
            }
            return ExprType::BOOL;
        }

        // Logical (&&, ||) results in BOOL
        if (binOp->op == "&&" || binOp->op == "||") {
            if (left != ExprType::BOOL || right != ExprType::BOOL) {
                throw std::runtime_error(
                    "Logical operations require BOOL operands");
            }
            return ExprType::BOOL;
        }
    }

    if (auto unaryOp = dynamic_cast<const UnaryOpNode *>(node)) {
        ExprType value = inferType(unaryOp->value.get());

        if (unaryOp->op == "+" || unaryOp->op == "-") {
            if (value != ExprType::INT) {
                std::runtime_error(
                    "Unary arithmetic operations require INT operand");
            }
            return ExprType::INT;
        }

        if (unaryOp->op == "!") {
            if (value != ExprType::BOOL) {
                std::runtime_error("Unary not operation requires BOOL operand");
            }
            return ExprType::BOOL;
        }
    }

    if (auto funcCall = dynamic_cast<const FunctionCallNode *>(node)) {
        auto it = m_functionTable.find(funcCall->name);
        if (it == m_functionTable.end()) {
            throw std::runtime_error("Undefined function: " + funcCall->name);
        }

        const FunctionSignature &sig = it->second;

        if (funcCall->arguments.size() != sig.paramTypes.size()) {
            throw std::runtime_error(
                "Function '" + funcCall->name + "' expects " +
                std::to_string(sig.paramTypes.size()) + " arguments, but got " +
                std::to_string(funcCall->arguments.size()));
        }

        for (size_t i = 0; i < funcCall->arguments.size(); ++i) {
            ExprType argType = inferType(funcCall->arguments[i].get());
            if (argType != sig.paramTypes[i]) {
                throw std::runtime_error(
                    "Type mismatch in argument " + std::to_string(i + 1) +
                    " of call to '" + funcCall->name + "'");
            }
        }

        return sig.returnType;
    }

    if (auto arrAcc = dynamic_cast<const ArrayAccessNode *>(node)) {
        if (m_symbolTable.find(arrAcc->name) == m_symbolTable.end()) {
            throw std::runtime_error("Undefined array: " + arrAcc->name);
        }

        if (auto indexNode =
                dynamic_cast<const NumberNode *>(arrAcc->index.get())) {
            int index = indexNode->value;
            int size = m_arrayTable[arrAcc->name];

            if (index < 0 || index >= size) {
                throw std::runtime_error(
                    "Array Index Out of Bounds: Index " +
                    std::to_string(index) + " is out of range for array '" +
                    arrAcc->name + "' of size " + std::to_string(size));
            }
        }

        return m_symbolTable[arrAcc->name];
    }

    return ExprType::UNKNOWN;
}

// Helper conversions
ExprType TypeChecker::tokenToType(const std::string &typeName) {
    if (typeName == "int")
        return ExprType::INT;
    if (typeName == "bool")
        return ExprType::BOOL;
    if (typeName == "void")
        return ExprType::VOID;
    if (typeName == "string")
        return ExprType::STRING;
    return ExprType::UNKNOWN;
}

std::string TypeChecker::typeToString(ExprType type) {
    switch (type) {
    case ExprType::INT:
        return "int";
    case ExprType::BOOL:
        return "bool";
    case ExprType::VOID:
        return "void";
    case ExprType::STRING:
        return "string";
    default:
        return "unknown";
    }
}
