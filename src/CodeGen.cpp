#include "include/CodeGen.hpp"
#include "include/AST.hpp"
#include <stdexcept>
#include <string>
#include <vector>

static const std::vector<std::string> ARG_REGS = {"rdi", "rsi", "rdx", "r8",
                                                  "r9"};

CodeGen::CodeGen(const std::string &filename) {
    m_out.open(filename);
    if (!m_out.is_open()) {
        throw std::runtime_error("Failed to open output file: " + filename);
    }
}

CodeGen::~CodeGen() {
    if (m_out.is_open())
        m_out.close();
}

void CodeGen::push(const std::string &reg) { emit("push " + reg); }
void CodeGen::pop(const std::string &reg) { emit("pop " + reg); }

void CodeGen::emit(const std::string &fmt) { m_out << "\t" << fmt << "\n"; }
void CodeGen::emitLabel(const std::string &label) { m_out << label << ":\n"; }

std::string CodeGen::getUniqueLabel(const std::string &prefix) {
    return "L_" + prefix + "_" + std::to_string(m_labelCounter++);
}

void CodeGen::generateProgram(const ProgramNode *program) {
    m_out << "section .text\n";
    m_out << "global main\n";
    m_out << "extern printf\n\n";

    for (const auto &func : program->functions) {
        generateFunction(func.get());
    }

    m_out << "\nsection .data\n";
    m_out << "fmt_int: db \"%d\", 10, 0\n";
    m_out << "fmt_str: db \"%s\", 10, 0\n";

    m_out << "bool_true: db \"true\", 0\n";
    m_out << "bool_false: db \"false\", 0\n";

    for (const auto &entry : m_rodata) {
        m_out << entry.label << ": db \"" << entry.value << "\", 0\n";
    }
}

void CodeGen::generateFunction(const FunctionDeclNode *node) {
    m_vars.clear();
    m_stackIndex = -8; // [rbp - 8]

    emitLabel(node->name);

    push("rbp");
    emit("mov rbp, rsp");

    for (size_t i = 0; i < node->parameters.size(); ++i) {
        std::string varName = node->parameters[i].name;
        std::string reg = ARG_REGS[i];

        m_vars[varName] = {m_stackIndex, node->parameters[i].type};

        push(reg);

        m_stackIndex -= 8;
    }

    generateBlock(node->body.get());

    emit("mov rax, 0");
    emit("mov rsp, rbp");
    pop("rbp");
    emit("ret");
    m_out << "\n";
}

void CodeGen::generateBlock(const BlockNode *node) {
    for (const auto &stmt : node->statements) {
        generate(stmt.get());

        if (dynamic_cast<const VarDeclNode *>(stmt.get())) {
            continue;
        }

        if (dynamic_cast<const IfNode *>(stmt.get()) ||
            dynamic_cast<const WhileNode *>(stmt.get()) ||
            dynamic_cast<const ReturnNode *>(stmt.get()) ||
            dynamic_cast<const BlockNode *>(stmt.get())) {
            continue;
        }

        pop("rax");
    }
}

void CodeGen::generate(const ASTNode *node) {
    // Expressions
    if (auto n = dynamic_cast<const NumberNode *>(node)) {
        emit("mov rax, " + std::to_string(n->value));
        push("rax");

    } else if (auto n = dynamic_cast<const BoolNode *>(node)) {
        emit("mov rax, " + std::to_string(n->value));
        push("rax");
    } else if (auto n = dynamic_cast<const StringNode *>(node)) {
        std::string label = makeStringLiteral(n->value);

        emit("lea rax, [rel " + label + "]");
        push("rax");
    } else if (auto n = dynamic_cast<const VariableNode *>(node)) {
        if (m_vars.find(n->name) == m_vars.end()) {
            throw std::runtime_error("Undefined variable: " + n->name);
        }
        int offset = m_vars[n->name].offset;
        emit("mov rax, [rbp" + std::to_string(offset) + "]");
        push("rax");
    } else if (auto n = dynamic_cast<const BinaryOpNode *>(node)) {
        genBinaryOp(n);
    } else if (auto n = dynamic_cast<const UnaryOpNode *>(node)) {
        genUnaryOp(n);
    } else if (auto n = dynamic_cast<const FunctionCallNode *>(node)) {
        genFunctionCall(n);
    }

    else if (auto n = dynamic_cast<const BlockNode *>(node)) {
        generateBlock(n);
    }

    // Statements
    else if (auto n = dynamic_cast<const VarDeclNode *>(node)) {
        genVarDecl(n);
    } else if (auto n = dynamic_cast<const VarAssignNode *>(node)) {
        genVarAssign(n);
    } else if (auto n = dynamic_cast<const ReturnNode *>(node)) {
        generate(
            n->returnValue.get()); // Calculate return value (result on stack)
        pop("rax");           // Move result to RAX (standard return register)
        emit("mov rsp, rbp"); // Epilogue
        pop("rbp");
        emit("ret");
    } else if (auto n = dynamic_cast<const IfNode *>(node)) {
        genIf(n);
    } else if (auto n = dynamic_cast<const WhileNode *>(node)) {
        genWhile(n);
    } else if (auto n = dynamic_cast<const ForNode *>(node)) {
        genFor(n);
    }
}

void CodeGen::genVarDecl(const VarDeclNode *node) {
    generate(node->initExpr.get());
    m_vars[node->name] = {m_stackIndex, node->type};
    m_stackIndex -= 8;
}

void CodeGen::genVarAssign(const VarAssignNode *node) {
    if (m_vars.find(node->name) == m_vars.end()) {
        throw std::runtime_error("CodeGen: Undeclared variable");
    }

    int offset = m_vars[node->name].offset;

    generate(node->newExpr.get());

    pop("rax");
    if (offset < 0) {

        emit("mov [rbp" + std::to_string(offset) + "], rax");
    } else {
        emit("mov [rbp-" + std::to_string(offset) + "], rax");
    }
    push("rax");
}

void CodeGen::genBinaryOp(const BinaryOpNode *node) {
    generate(node->left.get());
    generate(node->right.get());

    pop("rbx");
    pop("rax");

    std::string op = node->op;

    if (op == "+") {
        emit("add rax, rbx");
        push("rax");
    } else if (op == "-") {
        emit("sub rax, rbx");
        push("rax");
    } else if (op == "*") {
        emit("imul rax, rbx");
        push("rax");
    } else if (op == "/") {
        emit("cqo");
        emit("idiv rbx");
        push("rax");
    }
    // Comparisons
    else {
        emit("cmp rax, rbx");
        std::string setInstruction;

        if (op == "==")
            setInstruction = "sete al";
        else if (op == "!=")
            setInstruction = "setne al";
        else if (op == "<")
            setInstruction = "setl al";
        else if (op == ">")
            setInstruction = "setg al";
        else if (op == "<=")
            setInstruction = "setle al";
        else if (op == ">=")
            setInstruction = "setge al";

        emit(setInstruction);
        emit("movzx rax, al");
        push("rax");
    }
}

void CodeGen::genUnaryOp(const UnaryOpNode *node) {
    generate(node->value.get());

    pop("rax");

    std::string op = node->op;

    if (op == "+") {
        push("rax");
    } else if (op == "-") {
        emit("neg rax");
        push("rax");
    } else if (op == "!") {
        emit("cmp rax, 0");
        emit("sete al");
        emit("movzx rax, al");
        push("rax");
    }
}
void CodeGen::genIf(const IfNode *node) {
    std::string labelElse = getUniqueLabel("else");
    std::string labelEnd = getUniqueLabel("end");

    generate(node->condition.get());
    pop("rax");
    emit("cmp rax, 0");
    emit("je " + labelElse);

    generate(node->thenBranch.get());
    emit("jmp " + labelEnd);

    emitLabel(labelElse);
    if (node->elseBranch) {
        generate(node->elseBranch.get());
    }

    emitLabel(labelEnd);
}

void CodeGen::genWhile(const WhileNode *node) {
    std::string labelStart = getUniqueLabel("loop_start");
    std::string labelEnd = getUniqueLabel("loop_end");

    emitLabel(labelStart);

    generate(node->condition.get());
    pop("rax");
    emit("cmp rax, 0");
    emit("je " + labelEnd);

    generate(node->body.get());
    emit("jmp " + labelStart);

    emitLabel(labelEnd);
}

void CodeGen::genFor(const ForNode *node) {
    if (node->init) {
        generate(node->init.get());
    }

    std::string labelStart = getUniqueLabel("for_start");
    std::string labelEnd = getUniqueLabel("for_end");
    std::string labelUpdate = getUniqueLabel("for_update");

    emitLabel(labelStart);

    if (node->condition) {
        generate(node->condition.get());
        pop("rax");
        emit("cmp rax, 0");
        emit("je " + labelEnd);
    }

    generate(node->body.get());

    emitLabel(labelUpdate);
    if (node->update) {
        generate(node->update.get());

        pop("rax");
    }

    emit("jmp " + labelStart);
    emitLabel(labelEnd);
}

void CodeGen::genFunctionCall(const FunctionCallNode *node) {
    if (node->name == "print") {
        genPrintCall(node);
        return;
    }

    for (size_t i = 0; i < node->arguments.size(); ++i) {
        generate(node->arguments[i].get());
    }

    for (int i = node->arguments.size() - 1; i >= 0; --i) {
        pop(ARG_REGS[i]);
    }

    emit("call " + node->name);
    push("rax");
}

void CodeGen::genPrintCall(const FunctionCallNode *node) {
    if (node->arguments.empty())
        return;

    ASTNode *arg = node->arguments[0].get();

    if (auto strNode = dynamic_cast<const StringNode *>(arg)) {
        std::string label = makeStringLiteral(strNode->value);
        emit("lea rdi, [rel fmt_str]");
        emit("lea rsi, [rel " + label + "]");
        emit("mov rax, 0");
        emit("call printf WRT ..plt");
    } else if (auto boolNode = dynamic_cast<const BoolNode *>(arg)) {
        std::string boolLabel = boolNode->value ? "bool_true" : "bool_false";
        emit("lea rdi, [rel fmt_str]");
        emit("lea rsi, [rel " + boolLabel + "]");
        emit("mov rax, 0");
        emit("call printf WRT ..plt");
    } else if (auto varNode = dynamic_cast<VariableNode *>(arg)) {
        if (m_vars.find(varNode->name) == m_vars.end()) {
            throw std::runtime_error("CodeGen: Undefined variable");
        }

        VarInfo info = m_vars[varNode->name];

        if (info.type == "bool") {
            generate(arg);
            pop("rax");

            std::string labelIsFalse = getUniqueLabel("print_bool_false");
            std::string labelIsDone = getUniqueLabel("print_bool_done");

            emit("cmp rax, 0");
            emit("je " + labelIsFalse);

            emit("lea rsi, [rel bool_true]");
            emit("jmp " + labelIsDone);

            emitLabel(labelIsFalse);
            emit("lea rsi, [rel bool_false]");

            emitLabel(labelIsDone);
            emit("lea rdi, [rel fmt_str]");
            emit("mov rax, 0");
            emit("call printf WRT ..plt");
        } else if (info.type == "string") {
            generate(arg);

            pop("rsi");
            emit("lea rdi, [rel fmt_str]");
            emit("mov rax, 0");
            emit("call printf WRT ..plt");
        } else {
            generate(arg);
            pop("rsi");
            emit("lea rdi, [rel fmt_int]");
            emit("mov rax, 0");
            emit("call printf WRT ..plt");
        }
    } else {
        generate(arg);
        pop("rsi");
        emit("lea rdi, [rel fmt_int]");
        emit("mov rax, 0");
        emit("call printf WRT ..plt");
    }

    emit("push 0");
    return;
}

std::string CodeGen::makeStringLiteral(const std::string &value) {
    std::string label = "MSG_" + std::to_string(m_rodata.size());
    m_rodata.push_back({label, value});
    return label;
}
