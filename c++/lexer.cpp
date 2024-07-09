#include <format>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>

#include "doctest.h"
#include "lexer.h"

std::vector<Token> Lexer::generate_tokens() {
    std::vector<Token> tokens;
    std::string char_buffer;
    char ch;

    const std::regex identifier("[a-zA-Z_0-9]");
    const std::regex constant("[0-9]");
    const std::regex whitespace("\\s");

    while (_stream.get(ch)) {
        std::string str_ch(1, ch);

        if (ch == ';') {
            Token token(char_buffer);
            Token semicolon(";");
            tokens.emplace_back(token);
            tokens.emplace_back(semicolon);
            char_buffer.clear();
        } else if (ch == '(') {
            Token token(char_buffer);
            Token lparen("(");
            tokens.emplace_back(token);
            tokens.emplace_back(lparen);
            char_buffer.clear();
        } else if (ch == ')') {
            Token token(char_buffer);
            Token rparen(")");
            tokens.emplace_back(token);
            tokens.emplace_back(rparen);
            char_buffer.clear();
        } else if (ch == '{') {
            Token token("{");
            tokens.emplace_back(token);
        } else if (ch == '}') {
            Token token("}");
            tokens.emplace_back(token);
        } else if (std::regex_match(str_ch, whitespace)) {
            if (!char_buffer.empty()) {
                Token token(char_buffer);
                tokens.emplace_back(token);
                char_buffer.clear();
            }
        } else if (std::regex_match(str_ch, constant)) {
            char_buffer.push_back(ch);
        } else if (std::regex_match(str_ch, identifier)) {
            std::string first_char(1, char_buffer[0]);
            if (std::regex_match(first_char, constant)) {
                char_buffer.push_back(ch);
                throw std::invalid_argument(std::format(
                    "syntax error: identifiers can't begin with a digit - {}",
                    char_buffer));
            }
            char_buffer.push_back(ch);
        } else {
            throw std::invalid_argument(std::format(
                "Lexical error - char: {} - buffer: {}", ch, char_buffer));
        }
    }
    return tokens;
}

TEST_CASE("identifiers can't start with a digit") {
    std::stringstream source("2foo");
    Lexer lex(source);
    REQUIRE_THROWS_WITH_AS(
        lex.generate_tokens(),
        "syntax error: identifiers can't begin with a digit - 2f",
        std::invalid_argument);
}
