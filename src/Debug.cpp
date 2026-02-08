#include "include/Debug.hpp"
#include "include/AST.hpp"
#include "include/Token.hpp"
#include <iostream>
#include <memory>

std::string tokenTypeToString(TokenType type) {
    switch (type) {
    case TokenType::INT:
        return "INT";
    case TokenType::STRING:
        return "STRING";
    case TokenType::VOID:
        return "VOID";
    case TokenType::BOOL:
        return "BOOL";

    case TokenType::TRUE:
        return "TRUE";
    case TokenType::FALSE:
        return "FALSE";

    case TokenType::RETURN:
        return "RETURN";
    case TokenType::IF:
        return "IF";
    case TokenType::ELSE:
        return "ELSE";
    case TokenType::WHILE:
        return "WHILE";
    case TokenType::FOR:
        return "FOR";
    case TokenType::BREAK:
        return "BREAK";
    case TokenType::CONTINUE:
        return "CONTINUE";

    case TokenType::IDENTIFIER:
        return "ID";
    case TokenType::INT_LIT:
        return "INT_LIT";
    case TokenType::STRING_LIT:
        return "INT_LIT";

    case TokenType::PLUS:
        return "PLUS";
    case TokenType::MINUS:
        return "MINUS";
    case TokenType::STAR:
        return "MULT";
    case TokenType::SLASH:
        return "DIV";
    case TokenType::ASSIGN:
        return "ASSIGN";

    case TokenType::EQ:
        return "EQ";
    case TokenType::NEQ:
        return "NEQ";
    case TokenType::LT:
        return "LT";
    case TokenType::GT:
        return "GT";
    case TokenType::LE:
        return "LE";
    case TokenType::GE:
        return "GE";
    case TokenType::AND:
        return "AND";
    case TokenType::OR:
        return "OR";
    case TokenType::NOT:
        return "NOT";

    case TokenType::LPAREN:
        return "LPAREN";
    case TokenType::RPAREN:
        return "RPAREN";
    case TokenType::LBRACE:
        return "LBRACE";
    case TokenType::RBRACE:
        return "RBRACE";
    case TokenType::SEMI:
        return "SEMI";

    case TokenType::TOK_EOF:
        return "TOK_EOF";
    default:
        return "\033[93mUNKNOWN\033[0m";
    }
}

void printTokens(const std::vector<Token> &tokens) {
    std::string bar(25, '-');
    std::cout << "------- LEXER OUTPUT -------\n";
    std::cout << bar << "\n";
    std::cout << "| TYPE            | VALUE \n";
    std::cout << bar << "\n";

    for (const auto &token : tokens) {
        std::string typeStr = tokenTypeToString(token.type);

        std::cout << "| " << typeStr;
        for (int i = 0; i < (16 - typeStr.length()); i++)
            std::cout << " ";

        std::cout << "| " << token.value << "\n";
    }
    std::cout << "----------------------------\n";
}

// Helper to print indentation
void printIndent(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
}

void printAST(const ASTNode *node, int indent) {
    if (!node)
        return;

    printIndent(indent);

    // --- Expressions ---
    if (auto n = dynamic_cast<const NumberNode *>(node)) {
        std::cout << "[Number] " << n->value << "\n";
    } else if (auto n = dynamic_cast<const StringNode *>(node)) {
        std::cout << "[String] " << n->value << "\n";
    } else if (auto n = dynamic_cast<const BreakNode *>(node)) {
        std::cout << "[Break]\n";
    } else if (auto n = dynamic_cast<const ContinueNode *>(node)) {
        std::cout << "[Continue]\n";
    } else if (auto n = dynamic_cast<const BoolNode *>(node)) {
        std::cout << "[Bool] " << ((n->value == 1) ? "true" : "false") << "\n";
    } else if (auto n = dynamic_cast<const VariableNode *>(node)) {
        std::cout << "[Variable] " << n->name << "\n";
    } else if (auto n = dynamic_cast<const VarAssignNode *>(node)) {
        std::cout << "[VarAssign] " << n->name << "\n";
        printAST(n->newExpr.get(), indent + 1);
    } else if (auto n = dynamic_cast<const BinaryOpNode *>(node)) {
        std::cout << "[BinaryOp] " << n->op << "\n";
        printAST(n->left.get(), indent + 1);
        printAST(n->right.get(), indent + 1);
    } else if (auto n = dynamic_cast<const UnaryOpNode *>(node)) {
        std::cout << "[UnaryOp] " << n->op << "\n";
        printAST(n->value.get(), indent + 1);
    } else if (auto n = dynamic_cast<const FunctionCallNode *>(node)) {
        std::cout << "[Call] " << n->name << "()\n";
        for (const auto &arg : n->arguments) {
            printAST(arg.get(), indent + 1);
        }
    }

    // --- Statements ---
    else if (auto n = dynamic_cast<const VarDeclNode *>(node)) {
        std::cout << "[VarDecl] " << n->type << " " << n->name << "\n";
        if (n->initExpr) {
            printAST(n->initExpr.get(), indent + 1);
        }
    } else if (auto n = dynamic_cast<const ReturnNode *>(node)) {
        std::cout << "[Return]\n";
        printAST(n->returnValue.get(), indent + 1);
    } else if (auto n = dynamic_cast<const BlockNode *>(node)) {
        std::cout << "[Block]\n";
        for (const auto &stmt : n->statements) {
            printAST(stmt.get(), indent + 1);
        }
    } else if (auto n = dynamic_cast<const IfNode *>(node)) {
        std::cout << "[If]\n";
        printIndent(indent + 1);
        std::cout << "Condition:\n";
        printAST(n->condition.get(), indent + 2);

        printIndent(indent + 1);
        std::cout << "Then:\n";
        printAST(n->thenBranch.get(), indent + 2);

        if (n->elseBranch) {
            printIndent(indent + 1);
            std::cout << "Else:\n";
            printAST(n->elseBranch.get(), indent + 2);
        }
    } else if (auto n = dynamic_cast<const WhileNode *>(node)) {
        std::cout << "[While]\n";
        printIndent(indent + 1);
        std::cout << "Condition:\n";
        printAST(n->condition.get(), indent + 2);
        printAST(n->body.get(), indent + 1);
    } else if (auto n = dynamic_cast<const ForNode *>(node)) {
        std::cout << "[For]\n";
        printIndent(indent + 1);

        std::cout << "Init:\n";
        printAST(n->init.get(), indent + 2);
        printIndent(indent + 1);

        std::cout << "Condition:\n";
        printAST(n->condition.get(), indent + 2);
        printIndent(indent + 1);

        std::cout << "Update:\n";
        printAST(n->update.get(), indent + 2);
        printAST(n->body.get(), indent + 1);

    }

    // --- Functions & Root ---
    else if (auto n = dynamic_cast<const FunctionDeclNode *>(node)) {
        std::cout << "[Function] " << n->returnType << " " << n->name << "(";
        for (size_t i = 0; i < n->parameters.size(); ++i) {
            std::cout << n->parameters[i].type << " " << n->parameters[i].name;
            if (i < n->parameters.size() - 1)
                std::cout << ", ";
        }
        std::cout << ")\n";
        printAST(n->body.get(), indent + 1);
    } else if (auto n = dynamic_cast<const ProgramNode *>(node)) {
        std::cout << "[Program]\n";
        for (const auto &func : n->functions) {
            printAST(func.get(), indent + 1);
        }
    } else {
        std::cout << "\033[93m[Unknown Node]\033[0m\n";
    }
}
