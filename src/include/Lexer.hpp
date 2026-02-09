#pragma once
#include "Token.hpp"
#include <cstddef>
#include <string>
#include <vector>

class Lexer {
  public:
    Lexer(const std::string &source);

    std::vector<Token> tokenize();

  private:
    std::string m_src;
    size_t m_pos;
    int m_col;
    int m_row;

    char peek(const int offset = 0) const;
    char consume();
    void skipWhitespace();
    bool skipComments();
};
