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
                                      _tokens[_current_token].value()));
    }

    Program ast(std::move(fn));
    return ast;
}

void Parser::expect(std::string expected) {
    if (_current_token == _tokens.size()) {
        throw SyntaxError(std::format("Missing \"{}\"", expected));
    }

    const Token &actual = _tokens[_current_token++];
    if (actual.value() != expected) {
        throw SyntaxError(std::format("Expected \"{}\" but got \"{}\"",
                                      expected, actual.value()));
    }
}

std::unique_ptr<Function> Parser::parse_function() {
    expect("int");
    const Token &name = _tokens[_current_token++];
    if (name.type() != TokenType::Identifier) {
        throw SyntaxError(
            std::format("Invalid function name: {}", name.value()));
    }
    expect("(");
    expect("void");
    expect(")");
    expect("{");
    auto statement = parse_statement();
    expect("}");
    return std::make_unique<Function>(name.value(), std::move(statement));
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
    if (token.type() == TokenType::Constant) {
        return std::make_unique<Constant>(token.value());
    } else if (token.value() == "~") {
        return std::make_unique<Unary>(std::make_unique<Complement>(),
                                       parse_exp());
    } else if (token.value() == "-") {
        return std::make_unique<Unary>(std::make_unique<Negate>(), parse_exp());
    } else if (token.value() == "(") {
        auto exp = parse_exp();
        expect(")");
        return exp;
    } else {
        throw SyntaxError(std::format("Invalid expression: {}", token.value()));
    }
}

//// TESTS ////

TEST_CASE("Parser::parse_exp for decrement") {
    Parser parser(std::vector<Token>(
        {{"--", TokenType::Decrement}, {"100", TokenType::Constant}}));
    REQUIRE_THROWS_WITH_AS(parser.parse_exp(), "Invalid expression: --",
                           SyntaxError);
}

TEST_CASE("Parser::parse_exp for parenthesized expression") {
    Parser parser(std::vector<Token>(
        {{"~"}, {"("}, {"-"}, {"100", TokenType::Constant}, {")"}}));
    auto exp = parser.parse_exp();
    CHECK(exp->to_string() == "Complement(Negate(Constant(100)))");

    parser = Parser(std::vector<Token>({{"~"},
                                        {"("},
                                        {"("},
                                        {"("},
                                        {"-"},
                                        {"100", TokenType::Constant},
                                        {")"},
                                        {")"},
                                        {")"}}));
    CHECK(parser.parse_exp()->to_string() ==
          "Complement(Negate(Constant(100)))");

    parser = Parser(std::vector<Token>({
        {"~"},
        {"("},
        {"-"},
        {"100", TokenType::Constant},
    }));
    REQUIRE_THROWS_WITH_AS(parser.parse_exp(), "Missing \")\"", SyntaxError);
}

TEST_CASE("Parser::parse_exp for negation as a suffix") {
    Parser parser(std::vector<Token>{{"-"}});
    REQUIRE_THROWS_WITH_AS(parser.parse_exp(), "Invalid expression",
                           SyntaxError);
}

TEST_CASE("Parser::parse_exp for negation") {
    Parser parser(std::vector<Token>({{"-"}, {"100", TokenType::Constant}}));
    auto exp = parser.parse_exp();
    CHECK(exp->to_string() == "Negate(Constant(100))");
}

TEST_CASE("Parser::parse_exp for bitwise complement") {
    Parser parser(std::vector<Token>({{"~"}, {"100", TokenType::Constant}}));
    auto exp = parser.parse_exp();
    CHECK(exp->to_string() == "Complement(Constant(100))");
}

TEST_CASE("Parser::parse with extra tokens") {
    std::vector<Token> tokens{
        {"int"},    {"my_function", TokenType::Identifier},
        {"("},      {"void"},
        {")"},      {"{"},
        {"return"}, {"420", TokenType::Constant},
        {";"},      {"}"},
        {"foo"},    {"bar"}};
    Parser parser(tokens);
    REQUIRE_THROWS_WITH_AS(parser.parse(), "Unexpected token found: foo",
                           SyntaxError);
}

TEST_CASE("Parser::parse success") {
    std::vector<Token> tokens{
        {"int"},    {"my_function", TokenType::Identifier},
        {"("},      {"void"},
        {")"},      {"{"},
        {"return"}, {"420", TokenType::Constant},
        {";"},      {"}"}};
    Parser parser(tokens);
    auto ast = parser.parse();
    CHECK(ast.to_string() ==
          "Program(\n  Function(\n    name=\"my_function\",\n    "
          "body=Return(Constant(420))\n  )\n)");
}

TEST_CASE("Parser::parse_function success") {
    std::vector<Token> tokens{
        {"int"},    {"my_function", TokenType::Identifier},
        {"("},      {"void"},
        {")"},      {"{"},
        {"return"}, {"420", TokenType::Constant},
        {";"},      {"}"}};
    Parser parser(tokens);
    auto fn = parser.parse_function();
    CHECK(fn->to_string() == "Function(\n  name=\"my_function\",\n  "
                             "body=Return(Constant(420))\n)");
}

TEST_CASE("Parser::parse_function with missing token") {
    std::vector<Token> tokens{{"int"},
                              {"my_function", TokenType::Identifier},
                              {"void"},
                              {")"},
                              {"{"},
                              {"return"},
                              {"420", TokenType::Constant},
                              {";"},
                              {"}"}};
    Parser parser(tokens);
    REQUIRE_THROWS_WITH_AS(parser.parse_function(),
                           "Expected \"(\" but got \"void\"", SyntaxError);
}

TEST_CASE("Parser::parse_function with invalid name") {
    std::vector<Token> tokens{
        {"int"}, {"3"}, {"("},      {"void"},
        {")"},   {"{"}, {"return"}, {"420", TokenType::Constant},
        {";"},   {"}"}};
    Parser parser(tokens);
    REQUIRE_THROWS_WITH_AS(parser.parse_function(), "Invalid function name: 3",
                           SyntaxError);
}

TEST_CASE("Parser::parse_statement success") {
    std::vector<Token> tokens{{"return"}, {"1234", TokenType::Constant}, {";"}};
    Parser parser(tokens);
    auto stmt = parser.parse_statement();
    CHECK(stmt->to_string() == "Return(Constant(1234))");
}

TEST_CASE("Parser::parse_statement with out of order negation") {
    std::vector<Token> tokens{
        {"return"}, {"1234", TokenType::Constant}, {"-"}, {";"}};
    Parser parser(tokens);
    REQUIRE_THROWS_WITH_AS(parser.parse_statement(),
                           "Expected \";\" but got \"-\"", SyntaxError);
}

TEST_CASE("Parser::parse_statement error") {
    std::vector<Token> tokens{{"bork"}, {"1234", TokenType::Constant}, {";"}};
    Parser parser(tokens);
    REQUIRE_THROWS_WITH_AS(parser.parse_statement(),
                           "Expected \"return\" but got \"bork\"", SyntaxError);

    tokens = {{"return"}, {"1234", TokenType::Constant}};
    Parser parser2(tokens);
    REQUIRE_THROWS_WITH_AS(parser2.parse_statement(), "Missing \";\"",
                           SyntaxError);

    tokens = {{"return"}, {";"}};
    Parser parser3(tokens);
    REQUIRE_THROWS_WITH_AS(parser3.parse_statement(), "Invalid expression: ;",
                           SyntaxError);
}

TEST_CASE("Parser::parse_exp error") {
    Parser parser(std::vector<Token>({{"bark", TokenType::Identifier}}));
    REQUIRE_THROWS_WITH_AS(parser.parse_exp(), "Invalid expression: bark",
                           SyntaxError);
}

TEST_CASE("Parser::parse_exp") {
    Parser parser(std::vector<Token>({{"100", TokenType::Constant}}));
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
