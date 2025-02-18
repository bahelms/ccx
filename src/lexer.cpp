#include <algorithm>
#include <cctype>
#include <format>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "doctest.h"
#include "lexer.h"

[[nodiscard]] constexpr Reserved lookup_reserved(std::string_view keyword) {
    auto it = std::find_if(
        RESERVED_STRINGS.begin(), RESERVED_STRINGS.end(),
        [keyword](const auto &pair) { return pair.second == keyword; });

    if (it != RESERVED_STRINGS.end())
        return it->first;
    return Reserved::Unknown;
}

enum class State { Start, Identifier, Integer, Hyphen };

std::vector<Token> tokenize(std::istream &stream) {
    std::string buffer{};
    std::vector<Token> tokens{};
    State state{};
    char ch;

    while (stream.get(ch)) {
        switch (state) {
        case State::Start:
            if (std::isalpha(ch) || ch == '_') {
                buffer.push_back(ch);
                state = State::Identifier;
            } else if (std::isdigit(ch)) {
                buffer.push_back(ch);
                state = State::Integer;
            } else if (ch == '-') {
                state = State::Hyphen;
                buffer.push_back(ch);
            } else if (std::isspace(ch)) {
                // ignore
            } else {
                std::string kw(1, ch);
                tokens.emplace_back(lookup_reserved(kw));
            }
            break;

        case State::Identifier:
            if (std::isalnum(ch) || ch == '_') {
                buffer.push_back(ch);
            } else {
                auto reserved = lookup_reserved(buffer);
                if (reserved == Reserved::Unknown) {
                    tokens.emplace_back(Identifier{buffer});
                } else {
                    tokens.emplace_back(reserved);
                }
                buffer.clear();
                state = State::Start;
                stream.putback(ch);
            }
            break;

        case State::Integer:
            if (std::isdigit(ch)) {
                buffer.push_back(ch);
            } else {
                tokens.emplace_back(Integer{buffer});
                buffer.clear();
                state = State::Start;
                stream.putback(ch);
            }
            break;

        case State::Hyphen:
            if (ch == '-') {
                tokens.emplace_back(Reserved::Decrement);
                state = State::Start;
            } else {
                tokens.emplace_back(Reserved::Negate);
                state = State::Start;
                buffer.clear();
                stream.putback(ch);
            }
            break;

        default:
            throw SyntaxError("Unknown state");
        }
    }

    if (!buffer.empty()) {
        if (state == State::Integer) {
            tokens.emplace_back(Integer{buffer});
        } else if (state == State::Identifier) {
            tokens.emplace_back(Identifier{buffer});
        } else if (state == State::Hyphen) {
            tokens.emplace_back(Reserved::Negate);
        }
    }

    return tokens;
}

TEST_CASE("to_str works") {
    std::stringstream source("~(-2)");
    auto tokens = tokenize(source);
    CHECK(tokens[0].to_str() == "~");
    CHECK(tokens[1].to_str() == "(");
    CHECK(tokens[2].to_str() == "-");
    CHECK(tokens[3].to_str() == "2");
    CHECK(tokens[4].to_str() == ")");
}

TEST_CASE("out of order unarys") {
    std::stringstream source("2-");
    auto tokens = tokenize(source);
    CHECK(tokens.size() == 2);
    CHECK(std::get<Integer>(tokens[0].value) == "2");
    CHECK(std::get<Reserved>(tokens[1].value) == Reserved::Negate);
}

TEST_CASE("unarys with parens") {
    std::stringstream source("~(-2)");
    auto tokens = tokenize(source);
    CHECK(tokens.size() == 5);
    CHECK(std::get<Reserved>(tokens[0].value) == Reserved::Complement);
    CHECK(std::get<Reserved>(tokens[1].value) == Reserved::OpenParen);
    CHECK(std::get<Reserved>(tokens[2].value) == Reserved::Negate);
    CHECK(std::get<Integer>(tokens[3].value) == "2");
    CHECK(std::get<Reserved>(tokens[4].value) == Reserved::CloseParen);
}

TEST_CASE("unary stream") {
    std::stringstream source("-~--~");
    auto tokens = tokenize(source);
    CHECK(std::get<Reserved>(tokens[0].value) == Reserved::Negate);
    CHECK(std::get<Reserved>(tokens[1].value) == Reserved::Complement);
    CHECK(std::get<Reserved>(tokens[2].value) == Reserved::Decrement);
    CHECK(std::get<Reserved>(tokens[3].value) == Reserved::Complement);
}

TEST_CASE("decrement token") {
    std::stringstream source("--");
    auto tokens = tokenize(source);
    CHECK(std::get<Reserved>(tokens[0].value) == Reserved::Decrement);
}

TEST_CASE("bitwise complement token") {
    std::stringstream source("~");
    auto tokens = tokenize(source);
    CHECK(std::get<Reserved>(tokens[0].value) == Reserved::Complement);
}

TEST_CASE("hyphen token") {
    std::stringstream source("-");
    auto tokens = tokenize(source);
    CHECK(std::get<Reserved>(tokens[0].value) == Reserved::Negate);
}

TEST_CASE("identifiers can have digits") {
    std::stringstream source("i2x6(");
    auto tokens = tokenize(source);
    CHECK(std::get<Identifier>(tokens[0].value) == "i2x6");
    CHECK(std::get<Reserved>(tokens[1].value) == Reserved::OpenParen);
}

TEST_CASE("integer token") {
    std::stringstream source("2246;");
    auto tokens = tokenize(source);
    CHECK(std::get<Integer>(tokens[0].value) == "2246");
    CHECK(std::get<Reserved>(tokens[1].value) == Reserved::Semicolon);
}

TEST_CASE("whitespace is ignored") {
    std::stringstream source(" \n\t ;  ");
    auto tokens = tokenize(source);
    REQUIRE(tokens.size() == 1);
    CHECK(std::get<Reserved>(tokens[0].value) == Reserved::Semicolon);
}

TEST_CASE("simple valid program") {
    std::stringstream source("int \tmain(void)    {\n return 42; \n}");
    auto tokens = tokenize(source);
    REQUIRE(tokens.size() == 10);
    CHECK(std::get<Reserved>(tokens[0].value) == Reserved::IntType);
    CHECK(std::get<Identifier>(tokens[1].value) == "main");
    CHECK(std::get<Reserved>(tokens[2].value) == Reserved::OpenParen);
    CHECK(std::get<Reserved>(tokens[3].value) == Reserved::Void);
    CHECK(std::get<Reserved>(tokens[4].value) == Reserved::CloseParen);
    CHECK(std::get<Reserved>(tokens[5].value) == Reserved::OpenBrace);
    CHECK(std::get<Reserved>(tokens[6].value) == Reserved::Return);
    CHECK(std::get<Integer>(tokens[7].value) == "42");
    CHECK(std::get<Reserved>(tokens[8].value) == Reserved::Semicolon);
    CHECK(std::get<Reserved>(tokens[9].value) == Reserved::CloseBrace);
}
