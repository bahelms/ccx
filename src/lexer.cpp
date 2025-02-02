#include <format>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>

#include "doctest.h"
#include "lexer.h"

const std::regex identifier("^[a-zA-Z_]\\w*$");
const std::regex constant("^[0-9]+$");
const std::regex whitespace("\\s");

void flush_char_buffer(std::string &buffer, auto &tokens) {
    if (!buffer.empty()) {
        if (std::regex_match(buffer, constant)) {
            tokens.emplace_back(buffer, TokenType::Constant);
        } else if (std::regex_match(buffer, identifier)) {
            tokens.emplace_back(buffer, TokenType::Identifier);
        } else {
            throw SyntaxError(std::format(
                "Identifiers can't begin with a digit: {}", buffer));
        }
        buffer.clear();
    }
}

std::vector<Token> tokenize(std::istream &stream) {
    std::string char_buffer{};
    std::vector<Token> tokens{};
    char ch;
    while (stream.get(ch)) {
        std::string str_ch(1, ch);

        if (std::regex_match(str_ch, whitespace)) {
            flush_char_buffer(char_buffer, tokens);
        } else if (ch == '(' || ch == ')' || ch == ';' || ch == '~' ||
                   ch == '{' || ch == '}') {
            flush_char_buffer(char_buffer, tokens);
            tokens.emplace_back(Token(std::string(1, ch)));
        } else if (ch == '-') {
            flush_char_buffer(char_buffer, tokens);
            if (stream.peek() == '-') {
                stream.get(ch);
                tokens.emplace_back("--", TokenType::Decrement);
            } else {
                tokens.emplace_back(Token(std::string(1, ch)));
            }
        } else {
            char_buffer.push_back(ch);
        }
    }
    return tokens;
}

TEST_CASE("out of order unarys") {
    std::stringstream source("2-");
    auto tokens = tokenize(source);
    CHECK(tokens.size() == 2);
    CHECK(tokens[0].value() == "2");
    CHECK(tokens[1].value() == "-");
}

TEST_CASE("unarys with parens") {
    std::stringstream source("~(-2)");
    auto tokens = tokenize(source);
    CHECK(tokens.size() == 5);
    CHECK(tokens[0].value() == "~");
    CHECK(tokens[1].value() == "(");
    CHECK(tokens[2].value() == "-");
    CHECK(tokens[3].value() == "2");
    CHECK(tokens[4].value() == ")");
}

TEST_CASE("unary stream") {
    std::stringstream source("-~--~");
    auto tokens = tokenize(source);
    CHECK(tokens[0].value() == "-");
    CHECK(tokens[0].type() == TokenType::Literal);
    CHECK(tokens[1].value() == "~");
    CHECK(tokens[1].type() == TokenType::Literal);
    CHECK(tokens[2].value() == "--");
    CHECK(tokens[2].type() == TokenType::Decrement);
    CHECK(tokens[3].value() == "~");
    CHECK(tokens[3].type() == TokenType::Literal);
}

TEST_CASE("decrement token") {
    std::stringstream source("--");
    auto tokens = tokenize(source);
    CHECK(tokens[0].value() == "--");
    CHECK(tokens[0].type() == TokenType::Decrement);
}

TEST_CASE("bitwise complement token") {
    std::stringstream source("~");
    auto tokens = tokenize(source);
    CHECK(tokens[0].value() == "~");
    CHECK(tokens[0].type() == TokenType::Literal);
}

TEST_CASE("hyphen token") {
    std::stringstream source("-");
    auto tokens = tokenize(source);
    CHECK(tokens[0].value() == "-");
    CHECK(tokens[0].type() == TokenType::Literal);
}

TEST_CASE("identifiers can't start with a digit") {
    std::stringstream source("2foo;");
    REQUIRE_THROWS_WITH_AS(tokenize(source),
                           "Identifiers can't begin with a digit: 2foo",
                           SyntaxError);
}

TEST_CASE("identifiers can have digits") {
    std::stringstream source("i2x6(");
    auto tokens = tokenize(source);
    CHECK(tokens[0].value() == "i2x6");
    CHECK(tokens[0].type() == TokenType::Identifier);
    CHECK(tokens[1].value() == "(");
    CHECK(tokens[1].type() == TokenType::Literal);
}

TEST_CASE("constant token") {
    std::stringstream source("2246;");
    auto tokens = tokenize(source);
    const Token &constant = tokens[0];
    CHECK(constant.value() == "2246");
    CHECK(constant.type() == TokenType::Constant);
    CHECK(tokens[1].value() == ";");
}

TEST_CASE("whitespace is ignored") {
    std::stringstream source(" \n\t ;  ");
    auto tokens = tokenize(source);
    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].value() == ";");
}

TEST_CASE("simple valid program") {
    std::stringstream source("int \tmain(void)    {\n return 42; \n}");
    auto tokens = tokenize(source);
    REQUIRE(tokens.size() == 10);
}
