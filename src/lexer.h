#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <exception>
#include <fstream>
#include <map>
#include <string>
#include <string_view>
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

constexpr std::array<std::pair<Reserved, std::string_view>, 11>
    RESERVED_STRINGS{{
        {Reserved::IntType, "int"},
        {Reserved::Void, "void"},
        {Reserved::Return, "return"},
        {Reserved::OpenParen, "("},
        {Reserved::CloseParen, ")"},
        {Reserved::OpenBrace, "{"},
        {Reserved::CloseBrace, "}"},
        {Reserved::Semicolon, ";"},
        {Reserved::Negate, "-"},
        {Reserved::Decrement, "--"},
        {Reserved::Complement, "~"},
    }};

[[nodiscard]] constexpr std::string_view reserved_string(Reserved token) {
    auto it =
        std::find_if(RESERVED_STRINGS.begin(), RESERVED_STRINGS.end(),
                     [token](const auto &pair) { return pair.first == token; });
    return it != RESERVED_STRINGS.end() ? it->second : "Unknown Keyword";
}

// Container tokens
struct Identifier : std::string {};
struct Integer : std::string {};

template <typename... Ts> struct overloaded : Ts... {
    using Ts::operator()...; // Inherit operator() from all base classes
};

// Deduction guide to infer template arguments
template <typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

using TokenType = std::variant<Reserved, Identifier, Integer>;

struct Token {
    // Use class with constructors for better type safety?
    TokenType value{};

    [[nodiscard]] bool is(Reserved token) const noexcept {
        return is<Reserved>() && get_value<Reserved>() == token;
    }

    template <typename T> [[nodiscard]] bool is() const noexcept {
        return std::holds_alternative<T>(value);
    }

    template <typename T> [[nodiscard]] const T &get_value() const noexcept {
        assert(is<T>()); // TODO: disable in release build
        return std::get<T>(value);
    }

    std::string_view to_str() const {
        return std::visit(
            overloaded{
                [](Reserved token) { return reserved_string(token); },
                [](const Identifier &token) { return std::string_view(token); },
                [](const Integer &token) { return std::string_view(token); },
            },
            value);
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
