#include <format>
#include <iostream>
#include <regex>

#include "lexer.h"

std::vector<Token> Lexer::generate_tokens() {
    std::vector<Token> tokens;
    std::string char_buffer;
    char ch;

    const std::regex identifier("[a-zA-Z_0-9]");
    const std::regex constant("[0-9]");
    const std::regex whitespace("\\s");

    while (_stream.get(ch)) {
        std::cout << ch;
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
                Token token(char_buffer);
                tokens.emplace_back(token);
                char_buffer.clear();
            }
            char_buffer.push_back(ch);
        } else {
            throw std::invalid_argument(std::format(
                "Lexical error - char: {} - buffer: {}", ch, char_buffer));
        }
    }
    return tokens;
}
