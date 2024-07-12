#pragma once

#include <exception>
#include <fstream>
#include <string>
#include <vector>

enum class TokenType { Punct, Constant, Identifier };

class Token {
  public:
    Token() = default;
    Token(std::string v) : _value(v) {}
    Token(std::string v, TokenType t) : _value(v), _type(t) {}

    std::string value() const { return _value; }
    TokenType type() const { return _type; }

  private:
    std::string _value{};
    TokenType _type{};
};

class Lexer {
  public:
    Lexer(std::istream &s) : _stream(s) {}

    const std::vector<Token> &generate_tokens();

  private:
    std::istream &_stream;
    std::string _char_buffer{};
    std::vector<Token> _tokens;

    void flush_char_buffer();
};

class SyntaxError : public std::exception {
  public:
    SyntaxError(const std::string &msg) : _message(msg) {}

    const char *what() const noexcept override { return _message.c_str(); }

  private:
    std::string _message;
};
