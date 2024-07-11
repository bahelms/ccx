#pragma once

#include <format>
#include <memory>

#include "lexer.h"

class Exp {
  public:
    virtual ~Exp() = default;

    virtual std::string const to_string() = 0;
};

class Constant : public Exp {
    std::string _value;

  public:
    Constant(std::string v) : _value(v) {}

    std::string const to_string() override {
        return std::format("Constant({})", _value);
    }
};

class Statement {
  public:
    virtual ~Statement() = default;

    virtual std::string const to_string() = 0;
};

class Return : public Statement {
    std::unique_ptr<Exp> _exp;

  public:
    Return(auto e) : _exp(std::move(e)) {}

    std::string const to_string() {
        return std::format("Return(\n  {}\n)", _exp->to_string());
    }
};

class Function {
    std::string _name{};
    Statement &_body;

  public:
    Function(auto n, auto b) : _name(n), _body(b) {}

    std::string to_string() {
        return std::format("Function(\n  name={},\n  body={})", _name,
                           _body.to_string());
    }
};

class Program {
    Function _fn;

  public:
    Program(auto fn) : _fn(fn) {}

    std::string to_string() {
        return std::format("Program(\n  {}\n)", _fn.to_string());
    }
};

class Parser {
    std::vector<Token> _tokens;
    size_t _current_token{0};

    void expect(std::string);

  public:
    Parser(auto t) : _tokens(t) {}

    void parse();
    std::unique_ptr<Exp> parse_exp();
    std::unique_ptr<Statement> parse_statement();
};
