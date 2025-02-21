#include <iostream>
#include <memory>

#include "doctest.h"
#include "lexer.h"
#include "parser.h"

namespace Ast {
Program Parser::parse() {
    auto fn = parse_function();
    if (_current_token != _tokens.size()) {
        throw SyntaxError(std::format("Unexpected token found: {}",
                                      _tokens[_current_token].to_str()));
    }

    Program ast(std::move(fn));
    _current_token = 0; // for strategic benchmarking
    return ast;
}

void Parser::expect(std::string_view expected) {
    if (_current_token == _tokens.size()) {
        throw SyntaxError(std::format("Missing \"{}\"", expected));
    }

    const Token &actual = _tokens[_current_token++];
    if (actual.to_str() != expected) {
        throw SyntaxError(std::format("Expected \"{}\" but got \"{}\"",
                                      expected, actual.to_str()));
    }
}

std::unique_ptr<Function> Parser::parse_function() {
    expect("int");
    const Token &name = _tokens[_current_token++];
    if (!name.is<Identifier>()) {
        throw SyntaxError(
            std::format("Invalid function name: {}", name.to_str()));
    }
    expect("(");
    expect("void");
    expect(")");
    expect("{");
    auto statement = parse_statement();
    expect("}");
    return std::make_unique<Function>(name.to_str(), std::move(statement));
}

std::unique_ptr<Statement> Parser::parse_statement() {
    expect("return");
    auto exp = parse_exp();
    expect(";");
    return std::make_unique<Return>(std::move(exp));
}

std::unique_ptr<Exp> Parser::parse_exp() {
    if (_current_token >= _tokens.size()) {
        throw SyntaxError(std::format("Invalid expression"));
    }

    const Token &token = _tokens[_current_token++];
    if (token.is<Integer>()) {
        return std::make_unique<Constant>(token.to_str());
    } else if (token.is<Identifier>()) {
        throw SyntaxError(
            std::format("Invalid expression: {}", token.to_str()));
    }

    if (token.is(Reserved::Complement)) {
        return std::make_unique<Unary>(std::make_unique<Complement>(),
                                       parse_exp());
    } else if (token.is(Reserved::Negate)) {
        return std::make_unique<Unary>(std::make_unique<Negate>(), parse_exp());
    } else if (token.is(Reserved::OpenParen)) {
        auto exp = parse_exp();
        expect(")");
        return exp;
    }
    throw SyntaxError(std::format("Invalid expression: {}", token.to_str()));
}

//// TESTS ////

TEST_CASE("Parser::parse_exp for decrement") {
    Parser parser(
        std::vector<Token>({{Reserved::Decrement}, {Integer{"100"}}}));
    REQUIRE_THROWS_WITH_AS(parser.parse_exp(), "Invalid expression: --",
                           SyntaxError);
}

TEST_CASE("Parser::parse_exp for parenthesized expression") {
    Parser parser(std::vector<Token>({{Reserved::Complement},
                                      {Reserved::OpenParen},
                                      {Reserved::Negate},
                                      {Integer{"100"}},
                                      {Reserved::CloseParen}}));
    auto exp = parser.parse_exp();
    CHECK(exp->to_string() == "Complement(Negate(Constant(100)))");

    parser = Parser(std::vector<Token>({{Reserved::Complement},
                                        {Reserved::OpenParen},
                                        {Reserved::OpenParen},
                                        {Reserved::OpenParen},
                                        {Reserved::Negate},
                                        {Integer{"100"}},
                                        {Reserved::CloseParen},
                                        {Reserved::CloseParen},
                                        {Reserved::CloseParen}}));
    CHECK(parser.parse_exp()->to_string() ==
          "Complement(Negate(Constant(100)))");

    parser = Parser(std::vector<Token>({
        {Reserved::Complement},
        {Reserved::OpenParen},
        {Reserved::Negate},
        {Integer{"100"}},
    }));
    REQUIRE_THROWS_WITH_AS(parser.parse_exp(), "Missing \")\"", SyntaxError);
}

TEST_CASE("Parser::parse_exp for negation as a suffix") {
    Parser parser(std::vector<Token>{{Reserved::Negate}});
    REQUIRE_THROWS_WITH_AS(parser.parse_exp(), "Invalid expression",
                           SyntaxError);
}

TEST_CASE("Parser::parse_exp for negation") {
    Parser parser(std::vector<Token>({{Reserved::Negate}, {Integer{"100"}}}));
    auto exp = parser.parse_exp();
    CHECK(exp->to_string() == "Negate(Constant(100))");
}

TEST_CASE("Parser::parse_exp for bitwise complement") {
    Parser parser(
        std::vector<Token>({{Reserved::Complement}, {Integer{"100"}}}));
    auto exp = parser.parse_exp();
    CHECK(exp->to_string() == "Complement(Constant(100))");
}

TEST_CASE("Parser::parse with extra tokens") {
    std::vector<Token> tokens{
        {Reserved::IntType},    {Identifier{"my_function"}},
        {Reserved::OpenParen},  {Reserved::Void},
        {Reserved::CloseParen}, {Reserved::OpenBrace},
        {Reserved::Return},     {Integer{"420"}},
        {Reserved::Semicolon},  {Reserved::CloseBrace},
        {Identifier{"foo"}},    {Identifier{"bar"}}};
    Parser parser(tokens);
    REQUIRE_THROWS_WITH_AS(parser.parse(), "Unexpected token found: foo",
                           SyntaxError);
}

TEST_CASE("Parser::parse success") {
    std::vector<Token> tokens{
        {Reserved::IntType},    {Identifier{"my_function"}},
        {Reserved::OpenParen},  {Reserved::Void},
        {Reserved::CloseParen}, {Reserved::OpenBrace},
        {Reserved::Return},     {Integer{"420"}},
        {Reserved::Semicolon},  {Reserved::CloseBrace}};
    Parser parser(tokens);
    auto ast = parser.parse();
    CHECK(ast.to_string() ==
          "Program(\n  Function(\n    name=\"my_function\",\n    "
          "body=Return(Constant(420))\n  )\n)");
}

TEST_CASE("Parser::parse_function success") {
    std::vector<Token> tokens{
        {Reserved::IntType},    {Identifier{"my_function"}},
        {Reserved::OpenParen},  {Reserved::Void},
        {Reserved::CloseParen}, {Reserved::OpenBrace},
        {Reserved::Return},     {Integer{"420"}},
        {Reserved::Semicolon},  {Reserved::CloseBrace}};
    Parser parser(tokens);
    auto fn = parser.parse_function();
    CHECK(fn->to_string() == "Function(\n  name=\"my_function\",\n  "
                             "body=Return(Constant(420))\n)");
}

TEST_CASE("Parser::parse_function with missing token") {
    std::vector<Token> tokens{
        {Reserved::IntType},   {Identifier{"my_function"}},
        {Reserved::Void},      {Reserved::CloseParen},
        {Reserved::OpenBrace}, {Reserved::Return},
        {Integer{"420"}},      {Reserved::Semicolon},
        {Reserved::CloseBrace}};
    Parser parser(tokens);
    REQUIRE_THROWS_WITH_AS(parser.parse_function(),
                           "Expected \"(\" but got \"void\"", SyntaxError);
}

TEST_CASE("Parser::parse_function with invalid name") {
    std::vector<Token> tokens{{Reserved::IntType},    {Integer{"3"}},
                              {Reserved::OpenParen},  {Reserved::Void},
                              {Reserved::CloseParen}, {Reserved::OpenBrace},
                              {Reserved::Return},     {Integer{"420"}},
                              {Reserved::Semicolon},  {Reserved::CloseBrace}};
    Parser parser(tokens);
    REQUIRE_THROWS_WITH_AS(parser.parse_function(), "Invalid function name: 3",
                           SyntaxError);
}

TEST_CASE("Parser::parse_statement success") {
    std::vector<Token> tokens{
        {Reserved::Return}, {Integer{"1234"}}, {Reserved::Semicolon}};
    Parser parser(tokens);
    auto stmt = parser.parse_statement();
    CHECK(stmt->to_string() == "Return(Constant(1234))");
}

TEST_CASE("Parser::parse_statement with out of order negation") {
    std::vector<Token> tokens{{Reserved::Return},
                              {Integer{"1234"}},
                              {Reserved::Negate},
                              {Reserved::Semicolon}};
    Parser parser(tokens);
    REQUIRE_THROWS_WITH_AS(parser.parse_statement(),
                           "Expected \";\" but got \"-\"", SyntaxError);
}

TEST_CASE("Parser::parse_statement error") {
    std::vector<Token> tokens{
        {Identifier{"bork"}}, {Integer{"1234"}}, {Reserved::Semicolon}};
    Parser parser(tokens);
    REQUIRE_THROWS_WITH_AS(parser.parse_statement(),
                           "Expected \"return\" but got \"bork\"", SyntaxError);

    tokens = {{Reserved::Return}, {Integer{"1234"}}};
    Parser parser2(tokens);
    REQUIRE_THROWS_WITH_AS(parser2.parse_statement(), "Missing \";\"",
                           SyntaxError);

    tokens = {{Reserved::Return}, {Reserved::Semicolon}};
    Parser parser3(tokens);
    REQUIRE_THROWS_WITH_AS(parser3.parse_statement(), "Invalid expression: ;",
                           SyntaxError);
}

TEST_CASE("Parser::parse_exp error") {
    Parser parser(std::vector<Token>({{Identifier{"bark"}}}));
    REQUIRE_THROWS_WITH_AS(parser.parse_exp(), "Invalid expression: bark",
                           SyntaxError);
}

TEST_CASE("Parser::parse_exp") {
    Parser parser(std::vector<Token>({{Integer{"100"}}}));
    auto exp = parser.parse_exp();
    CHECK(exp->to_string() == "Constant(100)");
}

TEST_CASE("expressions have string representations") {
    Constant exp{"42"};
    CHECK(exp.to_string() == "Constant(42)");
}

TEST_CASE("statements have string representations") {
    Return stmt(std::make_unique<Constant>("23"));
    CHECK(stmt.to_string() == "Return(Constant(23))");
}
} // namespace Ast
