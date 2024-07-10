#pragma once

#include <fstream>
#include <string>
#include <vector>

class Token {
  public:
    Token(std::string v) : _value(v){};

    std::string value() const { return _value; }

  private:
    std::string _value;
};

class Lexer {
  public:
    Lexer(std::istream &s) : _stream(s){};

    std::vector<Token> generate_tokens();

  private:
    std::istream &_stream;
    std::string char_buffer;
    std::vector<Token> tokens;

    void flush_char_buffer();
};
