#include "codegen.h"
#include "doctest.h"
#include <array>
#include <memory>

ASM::Program ASM::Codegen::generate_program(AST &ast) {
    Program program(parse_func_def(ast.fn()));
    return program;
}

std::unique_ptr<ASM::FunctionDef>
ASM::Codegen::parse_func_def(std::unique_ptr<Function> fn) {
    auto instrs = parse_instructions(fn->body());
    return std::make_unique<ASM::FunctionDef>(fn->name(), std::move(instrs));
}

std::vector<std::unique_ptr<ASM::Instruction>>
ASM::Codegen::parse_instructions(std::unique_ptr<Statement> stmt) {
    std::vector<std::unique_ptr<ASM::Instruction>> instrs{};

    // Parse a Return statement
    std::unique_ptr<Return> return_stmt(dynamic_cast<Return *>(stmt.release()));
    instrs.emplace_back(std::make_unique<ASM::Mov>(
        parse_operand(return_stmt->exp()), std::make_unique<Register>()));
    instrs.emplace_back(std::make_unique<ASM::Ret>());

    return instrs;
}

std::unique_ptr<ASM::Operand>
ASM::Codegen::parse_operand(std::unique_ptr<Exp> exp) {
    // cast to int here?
    std::unique_ptr<Constant> c(dynamic_cast<Constant *>(exp.release()));
    return std::make_unique<Imm>(c->value());
}

//// TESTS ////

TEST_CASE("generating a simple program") {
    auto stmt = std::make_unique<Return>(std::make_unique<Constant>("789"));
    auto fn = std::make_unique<Function>("main", std::move(stmt));
    AST ast(std::move(fn));

    ASM::Codegen gen;
    ASM::Program program = gen.generate_program(ast);
    auto fn_def = program.fn_def();
    auto &instrs = fn_def->instructions();

    CHECK(fn_def->name() == "main");
    CHECK(instrs.size() == 2);
    CHECK(instrs[0]->to_string() == "Mov(Imm(789), Register)");
    CHECK(instrs[1]->to_string() == "Ret");
}

TEST_CASE("parsing a function definition without arguments") {
    auto stmt = std::make_unique<Return>(std::make_unique<Constant>("789"));
    auto fn = std::make_unique<Function>("main", std::move(stmt));
    ASM::Codegen gen;
    auto fn_def = gen.parse_func_def(std::move(fn));
    auto &instrs = fn_def->instructions();

    CHECK(fn_def->name() == "main");
    CHECK(instrs.size() == 2);
    CHECK(instrs[0]->to_string() == "Mov(Imm(789), Register)");
    CHECK(instrs[1]->to_string() == "Ret");
}

TEST_CASE("parsing a return statement produces mov and ret instructions") {
    ASM::Codegen gen;
    auto exp = std::make_unique<Constant>("789");
    auto stmt = std::make_unique<Return>(std::move(exp));
    auto instrs = gen.parse_instructions(std::move(stmt));
    CHECK(instrs.size() == 2);
    CHECK(instrs[0]->to_string() == "Mov(Imm(789), Register)");
    CHECK(instrs[1]->to_string() == "Ret");
}

TEST_CASE("constants generate immediate values") {
    ASM::Codegen gen;
    auto operand = gen.parse_operand(std::make_unique<Constant>("42"));
    std::unique_ptr<ASM::Imm> imm(dynamic_cast<ASM::Imm *>(operand.release()));
    CHECK(imm->value() == "42");
}
