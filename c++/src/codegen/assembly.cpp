#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>

#include "../../doctest.h"
#include "assembly.h"

namespace Asm {
std::unique_ptr<Operand> parse_operand(std::unique_ptr<Tacky::Val> val) {
    // cast to int here?
    std::unique_ptr<Tacky::Constant> constant(
        dynamic_cast<Tacky::Constant *>(val.release()));
    return std::make_unique<Imm>(constant->value());
}

std::vector<std::unique_ptr<Instruction>>
parse_instruction(std::unique_ptr<Tacky::Instruction> instr) {
    std::vector<std::unique_ptr<Instruction>> asm_instrs{};

    // Parse a Return statement
    Tacky::Return *raw_ptr = dynamic_cast<Tacky::Return *>(instr.release());
    if (!raw_ptr) {
        throw std::runtime_error("Instruction is not a Tacky::Return");
    }

    std::unique_ptr<Tacky::Return> return_instr(raw_ptr);
    asm_instrs.emplace_back(
        std::make_unique<Mov>(parse_operand(std::move(return_instr->val())),
                              std::make_unique<Register>()));
    asm_instrs.emplace_back(std::make_unique<Ret>());

    return asm_instrs;
}

FunctionDef parse_func_def(std::unique_ptr<Tacky::Function> fn) {
    std::vector<std::unique_ptr<Instruction>> fn_instrs{};
    for (auto &instr : fn->body()) {
        auto instrs = parse_instruction(std::move(instr));
        fn_instrs.insert(fn_instrs.end(),
                         std::make_move_iterator(instrs.begin()),
                         std::make_move_iterator(instrs.end()));
    }
    return FunctionDef(fn->name(), std::move(fn_instrs));
}

Program generate_assembly(Tacky::Program &tacky_ir) {
    Program program(parse_func_def(std::move(tacky_ir.fn())));
    return program;
}
} // namespace Asm

//// TESTS ////

TEST_CASE("generating a simple program") {
    std::vector<std::unique_ptr<Tacky::Instruction>> tacky_instrs{};
    auto instr = std::make_unique<Tacky::Return>(
        std::make_unique<Tacky::Constant>("789"));
    tacky_instrs.emplace_back(std::move(instr));
    auto fn =
        std::make_unique<Tacky::Function>("main", std::move(tacky_instrs));

    Tacky::Program tacky_ir(std::move(fn));

    Asm::Program program = Asm::generate_assembly(tacky_ir);
    auto fn_def = program.fn_def();
    auto &instrs = fn_def.instructions();

    CHECK(fn_def.name() == "main");
    CHECK(instrs.size() == 2);
    CHECK(instrs[0]->to_string() == "movl $789, %eax");
    CHECK(instrs[1]->to_string() == "ret");
}

TEST_CASE("parsing a function definition without arguments") {
    std::vector<std::unique_ptr<Tacky::Instruction>> tacky_instrs{};
    auto instr = std::make_unique<Tacky::Return>(
        std::make_unique<Tacky::Constant>("789"));
    tacky_instrs.emplace_back(std::move(instr));
    auto fn =
        std::make_unique<Tacky::Function>("main", std::move(tacky_instrs));

    auto fn_def = Asm::parse_func_def(std::move(fn));
    auto &instrs = fn_def.instructions();

    CHECK(fn_def.name() == "main");
    CHECK(instrs.size() == 2);
    CHECK(instrs[0]->to_string() == "movl $789, %eax");
    CHECK(instrs[1]->to_string() == "ret");
}

TEST_CASE("parsing a return instruction produces mov and ret instructions") {

    auto val = std::make_unique<Tacky::Constant>("789");
    auto instr = std::make_unique<Tacky::Return>(std::move(val));
    auto instrs = Asm::parse_instruction(std::move(instr));
    CHECK(instrs.size() == 2);
    CHECK(instrs[0]->to_string() == "movl $789, %eax");
    CHECK(instrs[1]->to_string() == "ret");
}

TEST_CASE("constants generate immediate values") {
    auto operand = Asm::parse_operand(std::make_unique<Tacky::Constant>("789"));
    std::unique_ptr<Asm::Imm> imm(dynamic_cast<Asm::Imm *>(operand.release()));
    CHECK(imm->value() == "789");
}
