#include "include/AST.hpp"
#include "include/CodeGen.hpp"
#include "include/Debug.hpp"
#include "include/Lexer.hpp"
#include "include/Parser.hpp"
#include "include/Token.hpp"
#include "include/TypeChecker.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>
#include <vector>

std::string readFile(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open source file: " << filename
                  << std::endl;
        exit(1);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./mycc <path to file>" << std::endl;
    }

    std::string source_code = readFile(argv[1]);
    try {
        Lexer lexer(source_code);
        std::vector<Token> tokens = lexer.tokenize();

        printTokens(tokens);

        Parser parser(tokens);
        std::unique_ptr<ProgramNode> root = parser.parseProgram();

        printAST(root.get());

        TypeChecker checker;
        checker.checkProgram(root.get());

        std::cout << "--- Generating Assembly ---\n";
        CodeGen generator("output.asm");
        generator.generateProgram(root.get());

        std::cout << "Compilation finished. Written to output.asm\n";

    } catch (const std::exception &e) {
        std::cerr << "COMPILER ERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
