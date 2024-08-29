#pragma once

#include <algorithm>
#include <array>
#include <format>
#include <map>
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
    std::string _name{};

  public:
    Pseudo(std::string id) : _name(id) {}
    const std::string &name() const { return _name; }
    std::string const to_string() override {
        return std::format("Pseudo({})", _name);
    }
};

class Stack : public Operand {
    int _offset{};

  public:
    Stack(int offset) : _offset(offset) {}
    std::string const to_string() override {
        return std::format("Stack({})", _offset);
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
    Mov() = default;
    Mov(std::unique_ptr<Operand> s, std::unique_ptr<Operand> d)
        : _src(std::move(s)), _dst(std::move(d)) {}

    std::unique_ptr<Operand> &src() { return _src; }
    std::unique_ptr<Operand> &dst() { return _dst; }

    void set_src(std::unique_ptr<Operand> src) { _src = std::move(src); }
    void set_dst(std::unique_ptr<Operand> dst) { _dst = std::move(dst); }

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
    Unary() = default;
    Unary(std::unique_ptr<UnaryOperator> op, std::unique_ptr<Operand> dst)
        : _op(std::move(op)), _dst(std::move(dst)) {}

    std::unique_ptr<Operand> &dst() { return _dst; }
    std::unique_ptr<UnaryOperator> &op() { return _op; }

    void set_dst(std::unique_ptr<Operand> dst) { _dst = std::move(dst); }
    void set_op(std::unique_ptr<UnaryOperator> op) { _op = std::move(op); }

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

class Generator {
    int _stack_offset{};
    int _offset_byte_size = 4;
    std::map<std::string, int> _cache{};

  public:
    Program generate_assembly(Tacky::Program &);
    Program convert_tacky_to_assembly(Tacky::Program &);
    Program replace_pseudo_registers(Asm::Program &);

    FunctionDef parse_func_def(std::unique_ptr<Tacky::Function>);
    std::vector<std::unique_ptr<Instruction>>
        parse_instruction(std::unique_ptr<Tacky::Instruction>);
    std::unique_ptr<Operand> parse_operand(std::unique_ptr<Tacky::Val> &);
    std::unique_ptr<UnaryOperator>
        parse_unop(std::unique_ptr<Tacky::UnaryOperator>);

    int find_stack_offset(std::string);
};
} // namespace Asm
