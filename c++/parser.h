#pragma once

#include "doctest.h"
#include "lexer.h"

class Exp {};

class Constant : public Exp {
    Token _token{};

  public:
    Constant(Token t) : _token(t) {}
};

class Statement {};

class Return : public Statement {
    Exp _exp{};
};

class Function {
    Token _name{};
    Statement _body{};
};

class Program {
    Function _fn{};
};

/* class AST { */
/*     Program _program{}; */
/* }; */

class Parser {
  public:
    Parser(auto t) : _tokens(t) {}
    Program parse();

  private:
    std::vector<Token> _tokens;
};

TEST_CASE("an expression can be printed") { Constant num{"42"}; }
