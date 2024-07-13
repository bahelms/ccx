#include "lexer.h"
#include "parser.h"
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
    std::string const to_string() { return std::format("Imm({})", _value); }
};

class Register : Operand {
  public:
    std::string const to_string() { return "Register"; }
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
    Mov(auto s, auto d) : _src(std::move(s)), _dst(std::move(d)) {}
    std::string const to_string() {
        return std::format("Mov({}, {})", _src->to_string(), _dst->to_string());
    }
};

class Ret : public Instruction {
    std::string const to_string() { return "Ret"; }
};

class Function {
    std::string _name;
    std::vector<Instruction> _instructions;
};

class Program {
    Function _fn_def;
};

class Codegen {
    AST _ast{};

  public:
    Codegen() = default;
    Codegen(auto a) : _ast(a) {}

    std::unique_ptr<Operand> parse_operand(std::unique_ptr<Exp> exp);

    std::array<std::unique_ptr<Instruction>, 2>
    parse_instructions(std::unique_ptr<Return> &stmt);
};
} // namespace ASM
