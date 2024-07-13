#include "codegen.h"
#include "doctest.h"
#include <array>
#include <memory>

std::array<std::unique_ptr<ASM::Instruction>, 2>
ASM::Codegen::parse_instructions(std::unique_ptr<Return> &stmt) {
    std::array<std::unique_ptr<Instruction>, 2> instrs{std::make_unique<Mov>(
        parse_operand(stmt->exp()), std::make_unique<Register>())};
    return instrs;
}

std::unique_ptr<ASM::Operand>
ASM::Codegen::parse_operand(std::unique_ptr<Exp> exp) {
    // cast to int here?
    std::unique_ptr<Constant> c(dynamic_cast<Constant *>(exp.release()));
    return std::make_unique<Imm>(c->value());
}

//// TESTS ////

TEST_CASE("parsing a return statement produces mov and ret instructions") {
    ASM::Codegen gen;
    auto exp = std::make_unique<Constant>("789");
    auto stmt = std::make_unique<Return>(std::move(exp));
    auto instrs = gen.parse_instructions(stmt);
    CHECK(instrs.size() == 2);
    CHECK(instrs[0]->to_string() == "");
}

TEST_CASE("constants generate immediate values") {
    ASM::Codegen gen;
    auto operand = gen.parse_operand(std::make_unique<Constant>("42"));
    std::unique_ptr<ASM::Imm> imm(dynamic_cast<ASM::Imm *>(operand.release()));
    CHECK(imm->value() == "42");
}
