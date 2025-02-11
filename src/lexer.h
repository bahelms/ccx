#pragma once

#include <cassert>
#include <exception>
#include <fstream>
#include <map>
#include <string>
#include <variant>
#include <vector>

// ignored: whitespace, newlines, eof
enum class Reserved {
    // Keywords
    IntType,
    Void,
    Return,

    // Control
    OpenParen,
    CloseParen,
    OpenBrace,
    CloseBrace,
    Semicolon,

    // Operators
    Negate,
    Decrement,
    Complement,
};

template <typename T> constexpr std::string_view reserved_string(T token) {
    switch (token) {
    case Reserved::IntType:
        return "int";
    case Reserved::Void:
        return "void";
    case Reserved::Return:
        return "return";
    case Reserved::OpenParen:
        return "(";
    case Reserved::CloseParen:
        return ")";
    case Reserved::OpenBrace:
        return "{";
    case Reserved::CloseBrace:
        return "}";
    case Reserved::Semicolon:
        return ";";
    case Reserved::Negate:
        return "-";
    case Reserved::Decrement:
        return "--";
    case Reserved::Complement:
        return "~";
    }
}

// Container tokens
struct Identifier : std::string {};
struct Integer : std::string {};

using TokenType = std::variant<Reserved, Identifier, Integer>;

struct Token {
    TokenType value{};

    template <typename T> [[nodiscard]] bool is() const noexcept {
        return std::holds_alternative<T>(value);
    }

    template <typename T> [[nodiscard]] const T &get_value() const noexcept {
        assert(is<T>()); // TODO: disable in release build
        return std::get<T>(value);
    }

    std::string_view to_str() const {
        if (is<Reserved>()) {
            return reserved_string(std::get<Reserved>(value));
        } else if (is<Integer>()) {
            return std::get<Integer>(value);
        } else {
            return std::get<Identifier>(value);
        }
    }
};

std::vector<Token> tokenize(std::istream &);

class SyntaxError : public std::exception {
  public:
    SyntaxError(const std::string &msg) : _message(msg) {}

    const char *what() const noexcept override { return _message.c_str(); }

  private:
    std::string _message;
};
