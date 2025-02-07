#pragma once

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

// Container tokens
struct Identifier : std::string {};
struct Integer : std::string {};

using TokenType = std::variant<Reserved, Identifier, Integer>;

struct Token {
    TokenType value{};

    std::string_view to_str() const { return "TOKEN"; }

    template <typename T> bool is() const {
        return std::holds_alternative<T>(value);
    }
};

std::vector<Token> tokenize(std::istream &);

/* template <typename E> */
/* constexpr auto to_str() */
/* // template <typename E> */
/* /1* constexpr auto toStringView(E e) { *1/ */
/* /1*     switch(e) { *1/ */
/* /1*         case E::Red: return "Red"; *1/ */
/* /1*         case E::Green: return "Green"; *1/ */
/* /1*         case E::Blue: return "Blue"; *1/ */
/* /1*     } *1/ */
/* /1* } *1/ */

/* enum class OldTokenType { Literal, Constant, Identifier, Decrement }; */

/* class OldToken { */
/*   public: */
/*     OldToken() = default; */
/*     OldToken(std::string v) : _value(v) {} */
/*     OldToken(std::string v, OldTokenType t) : _value(v), _type(t) {} */

/*     std::string value() const { return _value; } */
/*     OldTokenType type() const { return _type; } */

/*   private: */
/*     std::string _value{}; */
/*     OldTokenType _type{}; */
/* }; */

class SyntaxError : public std::exception {
  public:
    SyntaxError(const std::string &msg) : _message(msg) {}

    const char *what() const noexcept override { return _message.c_str(); }

  private:
    std::string _message;
};
