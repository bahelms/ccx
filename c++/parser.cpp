#include <iostream>
#include <memory>

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
    const Token &actual = _tokens[_current_token++];
    if (actual.value() != expected) {
        throw SyntaxError(
            std::format("Expected {}, but got {}", expected, actual.value()));
    }
}

std::unique_ptr<Statement> Parser::parse_statement() {
    expect("return");
    auto exp = parse_exp();
    expect(";");
    return std::make_unique<Return>(std::move(exp));
}

std::unique_ptr<Exp> Parser::parse_exp() {
    const Token &token = _tokens[_current_token++];
    return std::make_unique<Constant>(token.value());
}

TEST_CASE("Parser::parse_statement") {
    std::vector<Token> tokens{{"return"}, {"1234"}, {";"}};
    Parser parser(tokens);
    auto stmt = parser.parse_statement();
    CHECK(stmt->to_string() == "Return(\n  Constant(1234)\n)");
}

TEST_CASE("Parser::parse_exp") {
    Parser parser(std::vector<Token>({{"100"}}));
    auto exp = parser.parse_exp();
    CHECK(exp->to_string() == "Constant(100)");
}

TEST_CASE("expressions have string representations") {
    Constant exp{"42"};
    CHECK(exp.to_string() == "Constant(42)");
}

TEST_CASE("statements have string representations") {
    Return stmt(std::make_unique<Constant>("23"));
    CHECK(stmt.to_string() == "Return(\n  Constant(23)\n)");
}
