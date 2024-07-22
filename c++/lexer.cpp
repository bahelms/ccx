#include <format>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>

#include "doctest.h"
#include "lexer.h"

const std::regex identifier("[a-zA-Z_]\\w*");
const std::regex constant("[0-9]+");
const std::regex whitespace("\\s");

const std::vector<Token> &Lexer::generate_tokens() {
    char ch;
    while (_stream.get(ch)) {
        std::string str_ch(1, ch);

        if (std::regex_match(str_ch, whitespace)) {
            flush_char_buffer();
        } else if (ch == ';') {
            flush_char_buffer();
            _tokens.emplace_back(Token(";"));
        } else if (ch == '(') {
            flush_char_buffer();
            _tokens.emplace_back(Token("("));
        } else if (ch == ')') {
            flush_char_buffer();
            _tokens.emplace_back(Token(")"));
        } else if (ch == '{') {
            _tokens.emplace_back(Token("{"));
        } else if (ch == '}') {
            _tokens.emplace_back(Token("}"));
        } else {
            _char_buffer.push_back(ch);
        }
    }
    return _tokens;
}

void Lexer::flush_char_buffer() {
    if (!_char_buffer.empty()) {
        if (std::regex_match(_char_buffer, constant)) {
            _tokens.emplace_back(_char_buffer, TokenType::Constant);
        } else if (std::regex_match(_char_buffer, identifier)) {
            _tokens.emplace_back(_char_buffer, TokenType::Identifier);
        } else {
            throw SyntaxError(std::format(
                "Identifiers can't begin with a digit: {}", _char_buffer));
        }
        _char_buffer.clear();
    }
}

TEST_CASE("identifiers can't start with a digit") {
    std::stringstream source("2foo;");
    Lexer lex(source);
    REQUIRE_THROWS_WITH_AS(lex.generate_tokens(),
                           "Identifiers can't begin with a digit: 2foo",
                           SyntaxError);
}

TEST_CASE("identifiers can have digits") {
    std::stringstream source("i2x6(");
    Lexer lex(source);
    auto tokens = lex.generate_tokens();
    CHECK(tokens[0].value() == "i2x6");
    CHECK(tokens[0].type() == TokenType::Identifier);
    CHECK(tokens[1].value() == "(");
    CHECK(tokens[1].type() == TokenType::Punct);
}

TEST_CASE("constant token") {
    std::stringstream source("2246;");
    Lexer lex(source);
    auto tokens = lex.generate_tokens();
    const Token &constant = tokens[0];
    CHECK(constant.value() == "2246");
    CHECK(constant.type() == TokenType::Constant);
    CHECK(tokens[1].value() == ";");
}

TEST_CASE("whitespace is ignored") {
    std::stringstream source(" \n\t ;  ");
    Lexer lex(source);
    auto tokens = lex.generate_tokens();
    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].value() == ";");
}

TEST_CASE("simple valid program") {
    std::stringstream source("int \tmain(void)    {\n return 42; \n}");
    Lexer lex(source);
    auto tokens = lex.generate_tokens();
    REQUIRE(tokens.size() == 10);
}
