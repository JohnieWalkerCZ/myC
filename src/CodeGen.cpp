#include "include/CodeGen.hpp"
#include "include/AST.hpp"
#include <iostream>
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
    m_out << "default rel\n\n";
    m_out << "section .text\n";
    m_out << "global main\n";
    m_out << "extern printf\n\n";

    for (const auto &func : program->functions) {
        generateFunction(func.get());
    }

    generateDivisionByZero();

    m_out << "\nsection .data\n";
    m_out << "fmt_int: db \"%d\", 10, 0\n";
    m_out << "fmt_str: db \"%s\", 10, 0\n";

    m_out << "bool_true: db \"true\", 0\n";
    m_out << "bool_false: db \"false\", 0\n";

    m_out << "div_error_msg: db \"Error: division by zero\", 10, 0\n";

    for (const auto &entry : m_rodata) {
        m_out << entry.label << ": db \"" << entry.value << "\", 0\n";
    }
}

void CodeGen::generateFunction(const FunctionDeclNode *node) {
    m_vars.clear();
    m_stackIndex = -8;

    emitLabel(node->name);

    push("rbp");
    emit("mov rbp, rsp");

    emit("sub rsp, 256");

    for (size_t i = 0; i < node->parameters.size(); ++i) {
        std::string varName = node->parameters[i].name;
        std::string reg = ARG_REGS[i];

        m_vars[varName] = {m_stackIndex, node->parameters[i].type};

        emit("mov [rbp" + std::to_string(m_stackIndex) + "], " + reg);

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

        if (dynamic_cast<const VarDeclNode *>(stmt.get()) ||
            dynamic_cast<const ArrayDeclNode *>(stmt.get())) {
            continue;
        }

        if (dynamic_cast<const IfNode *>(stmt.get()) ||
            dynamic_cast<const WhileNode *>(stmt.get()) ||
            dynamic_cast<const ForNode *>(stmt.get()) ||
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
    } else if (auto n = dynamic_cast<const BreakNode *>(node)) {
        genBreak(n);
    } else if (auto n = dynamic_cast<const ContinueNode *>(node)) {
        genContinue(n);
    } else if (auto n = dynamic_cast<const VariableNode *>(node)) {
        genVar(n);
    } else if (auto n = dynamic_cast<const ArrayAccessNode *>(node)) {
        genArrayAccess(n);
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
    } else if (auto n = dynamic_cast<const ArrayDeclNode *>(node)) {
        genArrayDecl(n);
    } else if (auto n = dynamic_cast<const VarAssignNode *>(node)) {
        genVarAssign(n);
    } else if (auto n = dynamic_cast<const ArrayAssignNode *>(node)) {
        genArrayAssign(n);
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

    pop("rax");

    m_vars[node->name] = {m_stackIndex, node->type};
    emit("mov [rbp" + std::to_string(m_stackIndex) + "], rax");

    m_stackIndex -= 8;
}

void CodeGen::genArrayDecl(const ArrayDeclNode *node) {
    int count = node->size;
    int bytesNeeded = count * 8;

    m_vars[node->name] = {m_stackIndex, "array_" + node->type};

    m_stackIndex -= bytesNeeded;
}
void CodeGen::genVarAssign(const VarAssignNode *node) {
    if (m_vars.find(node->name) == m_vars.end()) {
        throw std::runtime_error("CodeGen: Undeclared variable");
    }

    int offset = m_vars[node->name].offset;

    generate(node->newExpr.get());

    pop("rax");
    emit("mov [rbp" + std::to_string(offset) + "], rax");
    push("rax");
}

void CodeGen::genArrayAssign(const ArrayAssignNode *node) {
    if (m_vars.find(node->name) == m_vars.end()) {
        throw std::runtime_error("Undefined array: " + node->name);
    }

    int baseOffset = m_vars[node->name].offset;

    generate(node->value.get());

    generate(node->index.get());
    pop("rbx");

    emit("imul rbx, 8");

    emit("mov rcx, " + std::to_string(baseOffset));

    emit("sub rcx, rbx");

    emit("add rcx, rbp");

    pop("rax");

    emit("mov [rcx], rax");

    push("rax");
}

void CodeGen::genVar(const VariableNode *node) {
    if (m_vars.find(node->name) == m_vars.end()) {
        throw std::runtime_error("Undefined variable: " + node->name);
    }
    int offset = m_vars[node->name].offset;
    emit("mov rax, [rbp" + std::to_string(offset) + "]");
    push("rax");
}

void CodeGen::genArrayAccess(const ArrayAccessNode *node) {
    if (m_vars.find(node->name) == m_vars.end()) {
        throw std::runtime_error("Undefined array: " + node->name);
    }

    int baseOffset = m_vars[node->name].offset;

    generate(node->index.get());
    pop("rax");

    emit("imul rax, 8");

    emit("mov rcx, " + std::to_string(baseOffset));

    emit("sub rcx, rax");

    emit("add rcx, rbp");
    emit("mov rax, [rcx]");

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
        emit("cmp rbx, 0");
        emit("je division_by_zero");
        emit("cqo");
        emit("idiv rbx");
        push("rax");
    } else if (op == "%") {
        emit("cmp rbx, 0");
        emit("je division_by_zero");
        emit("cqo");
        emit("idiv rbx");
        emit("mov rax, rdx");
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
    std::string labelEnd = getUniqueLabel("if_end");

    generate(node->condition.get());
    pop("rax");
    emit("cmp rax, 0");

    if (node->elseBranch) {
        std::string labelElse = getUniqueLabel("if_else");
        emit("je " + labelElse);

        generate(node->thenBranch.get());
        emit("jmp " + labelEnd);

        emitLabel(labelElse);
        generate(node->elseBranch.get());
    } else {
        emit("je " + labelEnd);

        generate(node->thenBranch.get());
    }

    emitLabel(labelEnd);
}

void CodeGen::genWhile(const WhileNode *node) {
    std::string labelStart = getUniqueLabel("loop_start");
    std::string labelEnd = getUniqueLabel("loop_end");

    pushLoopContext(labelEnd, labelStart);

    emitLabel(labelStart);

    generate(node->condition.get());
    pop("rax");
    emit("cmp rax, 0");
    emit("je " + labelEnd);

    generate(node->body.get());
    emit("jmp " + labelStart);

    emitLabel(labelEnd);

    popLoopContext();
}

void CodeGen::genFor(const ForNode *node) {
    if (node->init) {
        generate(node->init.get());
    }

    std::string labelStart = getUniqueLabel("for_start");
    std::string labelEnd = getUniqueLabel("for_end");
    std::string labelUpdate = getUniqueLabel("for_update");

    pushLoopContext(labelEnd, labelUpdate);

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

    popLoopContext();
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

void CodeGen::pushLoopContext(const std::string &breakLabel,
                              const std::string &continueLabel) {
    m_loopStack.push_back({breakLabel, continueLabel});
}

void CodeGen::popLoopContext() {
    if (!m_loopStack.empty()) {
        m_loopStack.pop_back();
    }
}

void CodeGen::genBreak(const BreakNode *) {
    if (m_loopStack.empty()) {
        throw std::runtime_error("CodeGen: 'break' statement outside loop");
    }

    std::string breakLabel = m_loopStack.back().first;
    emit("jmp " + breakLabel);
}

void CodeGen::genContinue(const ContinueNode *) {
    if (m_loopStack.empty()) {
        throw std::runtime_error("CodeGen: 'continue' statement outside loop");
    }

    std::string continueLabel = m_loopStack.back().second;
    emit("jmp " + continueLabel);
}

void CodeGen::generateDivisionByZero() {
    m_out << "\n";
    m_out << "division_by_zero:\n";
    m_out << "\tlea rdi, [rel div_error_msg]\n";
    m_out << "\tmov rax, 0\n";
    m_out << "\tcall printf WRT ..plt\n";
    m_out << "\tmov rax, 1\n";
    m_out << "\tret\n";
}
