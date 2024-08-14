#include "../lexer.h"
#include "../parser.h"
#include <array>
#include <memory>

namespace ASM {
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
    std::string const to_string() { return std::format("${}", _value); }
};

class Register : public Operand {
  public:
    std::string const to_string() { return "%eax"; }
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

    std::string const to_string() {
        return std::format("movl {}, {}", _src->to_string(), _dst->to_string());
    }
};

class Ret : public Instruction {
    std::string const to_string() { return "ret"; }
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
    std::unique_ptr<FunctionDef> _fn_def{};

  public:
    Program(auto fn_def) : _fn_def(std::move(fn_def)) {}

    std::unique_ptr<FunctionDef> fn_def() { return std::move(_fn_def); }
};

Program generate_assembly(Ast::Program &);
} // namespace ASM
