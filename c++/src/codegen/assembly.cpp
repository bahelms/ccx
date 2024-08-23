#include <algorithm>
#include <array>
#include <format>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>

#include "../../doctest.h"
#include "assembly.h"

namespace Asm {
Program Generator::generate_assembly(Tacky::Program &tacky_ir) {
    Program program(parse_func_def(std::move(tacky_ir.fn())));
    return program;
}

FunctionDef Generator::parse_func_def(std::unique_ptr<Tacky::Function> fn) {
    std::vector<std::unique_ptr<Instruction>> fn_instrs{};
    for (auto &instr : fn->body()) {
        auto instrs = parse_instruction(std::move(instr));
        fn_instrs.insert(fn_instrs.end(),
                         std::make_move_iterator(instrs.begin()),
                         std::make_move_iterator(instrs.end()));
    }
    return FunctionDef(fn->name(), std::move(fn_instrs));
}

std::vector<std::unique_ptr<Instruction>>
Generator::parse_instruction(std::unique_ptr<Tacky::Instruction> instr) {
    std::vector<std::unique_ptr<Instruction>> asm_instrs{};

    if (auto *return_instr = dynamic_cast<Tacky::Return *>(instr.get())) {
        auto reg = std::make_unique<Reg>(std::make_unique<AX>());
        asm_instrs.emplace_back(std::make_unique<Mov>(
            parse_operand(return_instr->val()), std::move(reg)));
        asm_instrs.emplace_back(std::make_unique<Ret>());
    } else if (auto *unary_instr = dynamic_cast<Tacky::Unary *>(instr.get())) {
        std::unique_ptr<Tacky::Val> unary_dst = unary_instr->dst();
        std::unique_ptr<Tacky::Val> unary_src = unary_instr->src();

        auto mov = std::make_unique<Mov>(parse_operand(unary_src),
                                         parse_operand(unary_dst));
        asm_instrs.emplace_back(std::move(mov));
        auto unary = std::make_unique<Unary>(parse_unop(unary_instr->op()),
                                             parse_operand(unary_dst));
        asm_instrs.emplace_back(std::move(unary));
    } else {
        throw std::runtime_error("Unknown Instruction");
    }

    return asm_instrs;
}

std::unique_ptr<Operand>
Generator::parse_operand(std::unique_ptr<Tacky::Val> &operand) {
    if (auto *constant = dynamic_cast<Tacky::Constant *>(operand.get())) {
        return std::make_unique<Imm>(constant->value());
    } else if (auto *var = dynamic_cast<Tacky::Var *>(operand.get())) {
        return std::make_unique<Pseudo>(var->value());
    } else {
        throw std::runtime_error("Unknown Operand");
    }
}

std::unique_ptr<UnaryOperator>
Generator::parse_unop(std::unique_ptr<Tacky::UnaryOperator> op) {
    if (auto *comp = dynamic_cast<Tacky::Complement *>(op.get())) {
        return std::make_unique<Not>();
    } else {
        return std::make_unique<Neg>();
    }
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

    Asm::Generator gen;
    Asm::Program program = gen.generate_assembly(tacky_ir);
    auto fn_def = program.fn_def();
    auto &instrs = fn_def.instructions();

    CHECK(fn_def.name() == "main");
    CHECK(instrs.size() == 2);
    CHECK(instrs[0]->to_string() == "movl $789, Reg(AX)");
    CHECK(instrs[1]->to_string() == "ret");
}

TEST_CASE("parsing a function definition without arguments") {
    std::vector<std::unique_ptr<Tacky::Instruction>> tacky_instrs{};
    auto instr = std::make_unique<Tacky::Return>(
        std::make_unique<Tacky::Constant>("789"));
    tacky_instrs.emplace_back(std::move(instr));
    auto fn =
        std::make_unique<Tacky::Function>("main", std::move(tacky_instrs));

    Asm::Generator gen;
    auto fn_def = gen.parse_func_def(std::move(fn));
    auto &instrs = fn_def.instructions();

    CHECK(fn_def.name() == "main");
    CHECK(instrs.size() == 2);
    CHECK(instrs[0]->to_string() == "movl $789, Reg(AX)");
    CHECK(instrs[1]->to_string() == "ret");
}

TEST_CASE("parsing a unary complement instruction") {
    auto op = std::make_unique<Tacky::Complement>();
    auto src = std::make_unique<Tacky::Constant>("789");
    auto dst = std::make_unique<Tacky::Var>("tmp.0");
    auto unary = std::make_unique<Tacky::Unary>(std::move(op), std::move(src),
                                                std::move(dst));
    Asm::Generator gen;
    auto instrs = gen.parse_instruction(std::move(unary));
    CHECK(instrs.size() == 2);
    CHECK(instrs[0]->to_string() == "movl $789, Pseudo(tmp.0)");
    CHECK(instrs[1]->to_string() == "notl Pseudo(tmp.0)");
}

TEST_CASE("parsing a unary negate instruction") {
    auto op = std::make_unique<Tacky::Negate>();
    auto src = std::make_unique<Tacky::Constant>("789");
    auto dst = std::make_unique<Tacky::Var>("tmp.0");
    auto unary = std::make_unique<Tacky::Unary>(std::move(op), std::move(src),
                                                std::move(dst));
    Asm::Generator gen;
    auto instrs = gen.parse_instruction(std::move(unary));
    CHECK(instrs.size() == 2);
    CHECK(instrs[0]->to_string() == "movl $789, Pseudo(tmp.0)");
    CHECK(instrs[1]->to_string() == "negl Pseudo(tmp.0)");
}

TEST_CASE("parsing a return instruction produces mov and ret instructions") {
    auto val = std::make_unique<Tacky::Constant>("789");
    auto instr = std::make_unique<Tacky::Return>(std::move(val));
    Asm::Generator gen;
    auto instrs = gen.parse_instruction(std::move(instr));
    CHECK(instrs.size() == 2);
    CHECK(instrs[0]->to_string() == "movl $789, Reg(AX)");
    CHECK(instrs[1]->to_string() == "ret");
}

TEST_CASE("constants generate immediate values") {
    std::unique_ptr<Tacky::Val> constant =
        std::make_unique<Tacky::Constant>("42");
    Asm::Generator gen;
    auto operand = gen.parse_operand(constant);
    std::unique_ptr<Asm::Imm> imm(dynamic_cast<Asm::Imm *>(operand.release()));
    CHECK(imm->value() == "42");
}
