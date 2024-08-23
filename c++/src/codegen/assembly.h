#pragma once

#include <algorithm>
#include <array>
#include <format>
#include <memory>

#include "../lexer.h"
#include "../parser.h"
#include "../tacky.h"

namespace Asm {
class Operand {
  public:
    virtual ~Operand() = default;
    virtual std::string const to_string() = 0;
};

class Imm : public Operand {
    std::string _value{};

  public:
    Imm(std::string s) : _value(s) {}

    std::string value() const { return _value; }
    std::string const to_string() override {
        return std::format("${}", _value);
    }
};

class Pseudo : public Operand {
    std::string _identifier{};

  public:
    Pseudo(std::string id) : _identifier(id) {}
    std::string const to_string() override {
        return std::format("Pseudo({})", _identifier);
    }
};

class Register {
  public:
    virtual ~Register() = default;
    virtual std::string const to_string() = 0;
};

class AX : public Register {
  public:
    std::string const to_string() override { return "AX"; }
};

class R10 : public Register {
  public:
    std::string const to_string() override { return "R10"; }
};

class Reg : public Operand {
    std::unique_ptr<Register> _register;

  public:
    Reg(std::unique_ptr<Register> r) : _register(std::move(r)) {}
    std::string const to_string() override {
        return std::format("Reg({})", _register->to_string());
    }
};

class Instruction {
  public:
    virtual ~Instruction() = default;
    virtual std::string const to_string() = 0;
};

class Mov : public Instruction {
    std::unique_ptr<Operand> _src{};
    std::unique_ptr<Operand> _dst{};

  public:
    Mov(std::unique_ptr<Operand> s, std::unique_ptr<Operand> d)
        : _src(std::move(s)), _dst(std::move(d)) {}

    std::string const to_string() override {
        return std::format("movl {}, {}", _src->to_string(), _dst->to_string());
    }
};

class Ret : public Instruction {
    std::string const to_string() override { return "ret"; }
};

class UnaryOperator {
  public:
    virtual ~UnaryOperator() = default;
    virtual std::string const to_string() = 0;
};

class Not : public UnaryOperator {
  public:
    std::string const to_string() override { return "notl"; }
};

class Neg : public UnaryOperator {
  public:
    std::string const to_string() override { return "negl"; }
};

class Unary : public Instruction {
    std::unique_ptr<UnaryOperator> _op{};
    std::unique_ptr<Operand> _dst{};

  public:
    Unary(std::unique_ptr<UnaryOperator> op, std::unique_ptr<Operand> dst)
        : _op(std::move(op)), _dst(std::move(dst)) {}

    std::string const to_string() override {
        return std::format("{} {}", _op->to_string(), _dst->to_string());
    }
};

class FunctionDef {
    std::string _name{};
    std::vector<std::unique_ptr<Instruction>> _instructions{};

  public:
    FunctionDef() = default;
    FunctionDef(auto n, auto i) : _name(n), _instructions(std::move(i)) {}

    std::string name() { return _name; }
    std::vector<std::unique_ptr<Instruction>> &instructions() {
        return _instructions;
    }
};

class Program {
    FunctionDef _fn_def{};

  public:
    Program(auto fn_def) : _fn_def(std::move(fn_def)) {}

    FunctionDef fn_def() { return std::move(_fn_def); }
};

Program generate_assembly(Tacky::Program &);
} // namespace Asm
