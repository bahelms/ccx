#include "assembly.h"
#include "../../doctest.h"
#include <array>
#include <memory>

namespace Asm {
std::unique_ptr<Operand> parse_operand(std::unique_ptr<Ast::Exp> exp) {
    // cast to int here?
    std::unique_ptr<Ast::Constant> c(
        dynamic_cast<Ast::Constant *>(exp.release()));
    return std::make_unique<Imm>(c->value());
}

std::vector<std::unique_ptr<Instruction>>
parse_instructions(std::unique_ptr<Ast::Statement> stmt) {
    std::vector<std::unique_ptr<Instruction>> instrs{};

    // Parse a Return statement
    std::unique_ptr<Ast::Return> return_stmt(
        dynamic_cast<Ast::Return *>(stmt.release()));
    instrs.emplace_back(std::make_unique<Mov>(parse_operand(return_stmt->exp()),
                                              std::make_unique<Register>()));
    instrs.emplace_back(std::make_unique<Ret>());

    return instrs;
}
std::unique_ptr<FunctionDef>
parse_func_def(std::unique_ptr<Ast::Function> &fn) {
    auto instrs = parse_instructions(fn->body());
    return std::make_unique<FunctionDef>(fn->name(), std::move(instrs));
}

Program generate_assembly(Ast::Program &ast) {
    Program program(parse_func_def(ast.fn()));
    return program;
}
} // namespace Asm

//// TESTS ////

TEST_CASE("generating a simple program") {
    auto stmt =
        std::make_unique<Ast::Return>(std::make_unique<Ast::Constant>("789"));
    auto fn = std::make_unique<Ast::Function>("main", std::move(stmt));
    Ast::Program ast(std::move(fn));

    Asm::Program program = Asm::generate_assembly(ast);
    auto fn_def = program.fn_def();
    auto &instrs = fn_def->instructions();

    CHECK(fn_def->name() == "main");
    CHECK(instrs.size() == 2);
    CHECK(instrs[0]->to_string() == "movl $789, %eax");
    CHECK(instrs[1]->to_string() == "ret");
}

TEST_CASE("parsing a function definition without arguments") {
    auto stmt =
        std::make_unique<Ast::Return>(std::make_unique<Ast::Constant>("789"));
    auto fn = std::make_unique<Ast::Function>("main", std::move(stmt));
    auto fn_def = Asm::parse_func_def(fn);
    auto &instrs = fn_def->instructions();

    CHECK(fn_def->name() == "main");
    CHECK(instrs.size() == 2);
    CHECK(instrs[0]->to_string() == "movl $789, %eax");
    CHECK(instrs[1]->to_string() == "ret");
}

TEST_CASE("parsing a return statement produces mov and ret instructions") {
    auto exp = std::make_unique<Ast::Constant>("789");
    auto stmt = std::make_unique<Ast::Return>(std::move(exp));
    auto instrs = Asm::parse_instructions(std::move(stmt));
    CHECK(instrs.size() == 2);
    CHECK(instrs[0]->to_string() == "movl $789, %eax");
    CHECK(instrs[1]->to_string() == "ret");
}

TEST_CASE("constants generate immediate values") {
    auto operand = Asm::parse_operand(std::make_unique<Ast::Constant>("42"));
    std::unique_ptr<Asm::Imm> imm(dynamic_cast<Asm::Imm *>(operand.release()));
    CHECK(imm->value() == "42");
}
