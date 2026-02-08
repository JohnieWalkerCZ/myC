#pragma once
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Base class for all nodes
struct ASTNode {
    virtual ~ASTNode() = default;
};

using Node = std::unique_ptr<ASTNode>;
// --- Expressions (Things that have a value) ---

// Represents a number: "42"
struct NumberNode : public ASTNode {
    int value;
    NumberNode(int v) : value(v) {}
};

struct BoolNode : public ASTNode {
    int value;
    BoolNode(int v) : value(v) {}
};

struct BreakNode : public ASTNode {
    BreakNode() {}
};

struct ContinueNode : public ASTNode {
    ContinueNode() {}
};

// Represents a variable usage: "x"
struct VariableNode : public ASTNode {
    std::string name;
    VariableNode(const std::string &n) : name(n) {}
};

struct StringNode : public ASTNode {
    std::string value;
    StringNode(const std::string &v) : value(v) {};
};

// Represents operations: "left + right" or "left > right"
struct BinaryOpNode : public ASTNode {
    std::string op; // "+", "-", "==", "<"
    Node left;
    Node right;

    BinaryOpNode(std::string oper, Node l, Node r)
        : op(oper), left(std::move(l)), right(std::move(r)) {}
};

struct UnaryOpNode : public ASTNode {
    std::string op;
    Node value;

    UnaryOpNode(std::string op, Node value) : op(op), value(std::move(value)) {}
};

// --- Statements (Things that DO something) ---

// Represents: "{ Statement1; Statement2; }"
struct BlockNode : public ASTNode {
    std::vector<Node> statements;
};

// Represents: "return expression;"
struct ReturnNode : public ASTNode {
    Node returnValue;
    ReturnNode(Node val) : returnValue(std::move(val)) {}
};

// Represents: "int x = 5 + 2;"
struct VarDeclNode : public ASTNode {
    std::string type;
    std::string name;
    Node initExpr; // The "5 + 2" part

    VarDeclNode(std::string t, std::string n, Node init)
        : type(t), name(n), initExpr(std::move(init)) {}
};

struct VarAssignNode : public ASTNode {
    std::string name;
    Node newExpr;

    VarAssignNode(std::string n, Node init)
        : name(n), newExpr(std::move(init)) {}
};

// Represents: "if (cond) { ... } else { ... }"
struct IfNode : public ASTNode {
    Node condition;
    Node thenBranch;
    Node elseBranch; // Can be nullptr if no 'else'

    IfNode(Node cond, Node thn, Node els = nullptr)
        : condition(std::move(cond)), thenBranch(std::move(thn)),
          elseBranch(std::move(els)) {}
};

// Loops
struct WhileNode : public ASTNode {
    Node condition;
    Node body;

    WhileNode(Node cond, Node b)
        : condition(std::move(cond)), body(std::move(b)) {}
};

struct ForNode : public ASTNode {
    Node init;
    Node condition;
    Node update;
    Node body;

    ForNode(Node init, Node cond, Node update, Node body)
        : init(std::move(init)), condition(std::move(cond)),
          update(std::move(update)), body(std::move(body)) {};
};

struct FunctionCallNode : public ASTNode {
    std::string name;
    std::vector<Node> arguments;

    FunctionCallNode(std::string name, std::vector<Node> args)
        : name(std::move(name)), arguments(std::move(args)) {};
};

struct FunctionDeclNode : public ASTNode {
    std::string returnType;
    std::string name;

    struct Param {
        std::string type;
        std::string name;
    };
    std::vector<Param> parameters;

    std::unique_ptr<BlockNode> body;

    FunctionDeclNode(std::string rt, std::string n, std::vector<Param> params,
                     std::unique_ptr<BlockNode> b)
        : returnType(rt), name(n), parameters(params), body(std::move(b)) {}
};

struct ProgramNode : public ASTNode {
    std::vector<std::unique_ptr<FunctionDeclNode>> functions;
};
