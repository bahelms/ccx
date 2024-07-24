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

    auto value() const { return _value; }
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

    auto exp() { return std::move(_exp); }
    std::string const to_string() {
        return std::format("Return({})", _exp->to_string());
    }
};

class Function {
    std::string _name{};
    std::unique_ptr<Statement> _body;

  public:
    Function(auto n, auto b) : _name(n), _body(std::move(b)) {}

    std::string to_string(int indent = 0) {
        int next_lvl = indent + 2;
        return std::format(
            "Function(\n{:<{}}name=\"{}\",\n{:<{}}body={}\n{:<{}})", "",
            next_lvl, _name, "", next_lvl, _body->to_string(), "", indent);
    }

    std::unique_ptr<Statement> body() { return std::move(_body); }
    std::string name() { return _name; }
};

class AST {
    std::unique_ptr<Function> _fn;

  public:
    AST() = default;
    AST(auto fn) : _fn(std::move(fn)) {}

    std::string to_string() {
        return std::format("Program(\n  {}\n)", _fn->to_string(2));
    }
};

class Parser {
    std::vector<Token> _tokens;
    size_t _current_token{0};

    void expect(std::string);

  public:
    Parser(auto t) : _tokens(t) {}

    AST parse();
    std::unique_ptr<Exp> parse_exp();
    std::unique_ptr<Statement> parse_statement();
    std::unique_ptr<Function> parse_function();
};
