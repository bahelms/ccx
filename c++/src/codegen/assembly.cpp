#include "assembly.h"
#include "../../doctest.h"
#include <array>
#include <memory>

namespace ASM {
std::unique_ptr<Operand> parse_operand(std::unique_ptr<Exp> exp) {
    // cast to int here?
    std::unique_ptr<Constant> c(dynamic_cast<Constant *>(exp.release()));
    return std::make_unique<Imm>(c->value());
}

std::vector<std::unique_ptr<Instruction>>
parse_instructions(std::unique_ptr<Statement> stmt) {
    std::vector<std::unique_ptr<Instruction>> instrs{};

    // Parse a Return statement
    std::unique_ptr<Return> return_stmt(dynamic_cast<Return *>(stmt.release()));
    instrs.emplace_back(std::make_unique<Mov>(parse_operand(return_stmt->exp()),
                                              std::make_unique<Register>()));
    instrs.emplace_back(std::make_unique<Ret>());

    return instrs;
}
std::unique_ptr<FunctionDef> parse_func_def(std::unique_ptr<Function> fn) {
    auto instrs = parse_instructions(fn->body());
    return std::make_unique<FunctionDef>(fn->name(), std::move(instrs));
}

Program generate_assembly(AST &ast) {
    Program program(parse_func_def(ast.fn()));
    return program;
}
} // namespace ASM

//// TESTS ////

TEST_CASE("generating a simple program") {
    auto stmt = std::make_unique<Return>(std::make_unique<Constant>("789"));
    auto fn = std::make_unique<Function>("main", std::move(stmt));
    AST ast(std::move(fn));

    ASM::Program program = ASM::generate_assembly(ast);
    auto fn_def = program.fn_def();
    auto &instrs = fn_def->instructions();

    CHECK(fn_def->name() == "main");
    CHECK(instrs.size() == 2);
    CHECK(instrs[0]->to_string() == "movl $789, %eax");
    CHECK(instrs[1]->to_string() == "ret");
}

TEST_CASE("parsing a function definition without arguments") {
    auto stmt = std::make_unique<Return>(std::make_unique<Constant>("789"));
    auto fn = std::make_unique<Function>("main", std::move(stmt));
    auto fn_def = ASM::parse_func_def(std::move(fn));
    auto &instrs = fn_def->instructions();

    CHECK(fn_def->name() == "main");
    CHECK(instrs.size() == 2);
    CHECK(instrs[0]->to_string() == "movl $789, %eax");
    CHECK(instrs[1]->to_string() == "ret");
}

TEST_CASE("parsing a return statement produces mov and ret instructions") {
    auto exp = std::make_unique<Constant>("789");
    auto stmt = std::make_unique<Return>(std::move(exp));
    auto instrs = ASM::parse_instructions(std::move(stmt));
    CHECK(instrs.size() == 2);
    CHECK(instrs[0]->to_string() == "movl $789, %eax");
    CHECK(instrs[1]->to_string() == "ret");
}

TEST_CASE("constants generate immediate values") {
    auto operand = ASM::parse_operand(std::make_unique<Constant>("42"));
    std::unique_ptr<ASM::Imm> imm(dynamic_cast<ASM::Imm *>(operand.release()));
    CHECK(imm->value() == "42");
}
