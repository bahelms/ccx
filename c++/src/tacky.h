#pragma once

#include <algorithm>
#include <format>
#include <memory>
#include <vector>

#include "parser.h"

namespace Tacky {
class Val {
  public:
    virtual ~Val() = default;
    virtual std::string const value() = 0;
    virtual std::string const to_string() = 0;
};

class Constant : public Val {
    std::string _int{};

  public:
    Constant(std::string i) : _int(i) {}
    std::string const value() override { return _int; }
    std::string const to_string() override {
        return std::format("Constant({})", _int);
    }
};

class Var : public Val {
    std::string _identifier{};

  public:
    Var(std::string i) : _identifier(i) {}
    std::string const value() override { return _identifier; }
    std::string const to_string() override {
        return std::format("Var({})", _identifier);
    }
};

class UnaryOperator {
  public:
    virtual ~UnaryOperator() = default;
    virtual std::string const to_string() = 0;
};

class Complement : public UnaryOperator {
  public:
    std::string const to_string() override { return "Complement"; }
};

class Negate : public UnaryOperator {
  public:
    std::string const to_string() override { return "Negate"; }
};

class Instruction {
  public:
    virtual ~Instruction() = default;
    virtual std::string const to_string() = 0;
};

class Return : public Instruction {
    std::unique_ptr<Val> _val{};

  public:
    Return(std::unique_ptr<Val> v) : _val(std::move(v)) {}

    std::unique_ptr<Val> &val() { return _val; }
    std::string const to_string() override {
        return std::format("Return({})", _val->to_string());
    }
};

class Unary : public Instruction {
    std::unique_ptr<UnaryOperator> _op{};
    std::unique_ptr<Val> _src{};
    std::unique_ptr<Val> _dst{};

  public:
    Unary(std::unique_ptr<UnaryOperator> op, std::unique_ptr<Val> src,
          std::unique_ptr<Val> dst)
        : _op(std::move(op)), _src(std::move(src)), _dst(std::move(dst)) {}

    std::unique_ptr<UnaryOperator> op() { return std::move(_op); }

    std::string const to_string() override {
        return std::format("Unary({}, {}, {})", _op->to_string(),
                           _src->to_string(), _dst->to_string());
    }
};

class Function {
    std::string _name{};
    std::vector<std::unique_ptr<Instruction>> _body{};

  public:
    Function(std::string name, std::vector<std::unique_ptr<Instruction>> instrs)
        : _name(name), _body(std::move(instrs)) {}
    std::vector<std::unique_ptr<Instruction>> body() {
        return std::move(_body);
    }
    std::string name() { return _name; }
};

class Program {
    std::unique_ptr<Function> _fn{};

  public:
    Program(std::unique_ptr<Function> fn) : _fn(std::move(fn)) {}
    std::unique_ptr<Function> &fn() { return _fn; }
};

class Generator {
    int _temp_var_counter{};
    std::vector<std::unique_ptr<Instruction>> _instrs{};

    std::unique_ptr<UnaryOperator>
    convert_unop(std::unique_ptr<Ast::UnaryOperator> op);

    std::string temp_name() {
        return std::format("main.{}", _temp_var_counter++);
    }

  public:
    std::vector<std::unique_ptr<Instruction>> &instructions() {
        return _instrs;
    }

    Program convert_ast(Ast::Program &);
    std::unique_ptr<Function>
    convert_function(std::unique_ptr<Ast::Function> &);
    void convert_statement(std::unique_ptr<Ast::Statement>);
    std::unique_ptr<Val> convert_exp(std::unique_ptr<Ast::Exp>);
};
} // namespace Tacky
