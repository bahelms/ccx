#pragma once

#include <exception>
#include <fstream>
#include <string>
#include <vector>

enum class TokenType { Literal, Constant, Identifier, Decrement };

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

std::vector<Token> tokenize(std::istream &);

class SyntaxError : public std::exception {
  public:
    SyntaxError(const std::string &msg) : _message(msg) {}

    const char *what() const noexcept override { return _message.c_str(); }

  private:
    std::string _message;
};
