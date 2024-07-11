#include <iostream>

#include "doctest.h"
#include "lexer.h"
#include "parser.h"

void Parser::parse() {
  for (auto token : _tokens) {
    std::cout << "Token: " << token.value() << std::endl;
  }
  /* Program ast; */
  /* return ast; */
}

void Parser::expect(std::string expected) {
  Token &actual = _tokens[_current_token++];
  if (actual.value() != expected) {
    throw SyntaxError(
        std::format("Expected {}, but got {}", expected, actual.value()));
  }
}

Exp Parser::parse_exp() {
  Token &token = _tokens[_current_token++];
  return Constant(token.value());
}

TEST_CASE("Parser::parse_exp") {
  Parser parser(std::vector<Token>({{"100"}}));
  Exp exp = parser.parse_exp();
  CHECK(exp.value() == "100");
}

TEST_CASE("expressions have string representations") {
  Constant exp{"42"};
  CHECK(exp.to_string() == "Constant(42)");
}

TEST_CASE("statements have string representations") {
  Constant exp("23");
  Return stmt(exp);
  CHECK(stmt.to_string() == "Return(\n  Constant(23)\n)");
}
