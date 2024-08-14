#include <algorithm>
#include <memory>
#include <vector>

#include "../doctest.h"
#include "parser.h"
#include "tacky.h"

namespace Tacky {
std::unique_ptr<Program> Generator::convert_ast(Ast::AST &ast) {
    return std::make_unique<Program>(convert_function(ast.fn()));
}

std::unique_ptr<Function>
Generator::convert_function(std::unique_ptr<Ast::Function> &fn) {
    convert_statement(fn->body());
    return std::make_unique<Function>(fn->name(), std::move(_instrs));
}

void Generator::convert_statement(std::unique_ptr<Ast::Statement> stmt) {
    auto return_stmt = dynamic_cast<Ast::Return *>(stmt.release());
    auto val = convert_exp(return_stmt->exp());
    _instrs.emplace_back(std::make_unique<Return>(std::move(val)));
}

std::unique_ptr<Val> Generator::convert_exp(std::unique_ptr<Ast::Exp> exp) {
    if (auto *constant = dynamic_cast<Ast::Constant *>(exp.get())) {
        return std::make_unique<Constant>(constant->value());
    } else {
        auto unary(dynamic_cast<Ast::Unary *>(exp.get()));
        auto tacky_op = convert_unop(unary->op());
        auto src = convert_exp(unary->exp());
        auto name = temp_name();
        auto instr = std::make_unique<Unary>(
            std::move(tacky_op), std::move(src), std::make_unique<Var>(name));
        _instrs.emplace_back(std::move(instr));
        return std::make_unique<Var>(name);
    }
}

std::unique_ptr<UnaryOperator>
Generator::convert_unop(std::unique_ptr<Ast::UnaryOperator> op) {
    if (auto *comp = dynamic_cast<Ast::Complement *>(op.get())) {
        return std::make_unique<Complement>();
    } else {
        return std::make_unique<Negate>();
    }
}
} // namespace Tacky

//// TESTS ////

TEST_CASE("convert_ast") {
    auto unary =
        std::make_unique<Ast::Unary>(std::make_unique<Ast::Complement>(),
                                     std::make_unique<Ast::Constant>("123"));
    auto stmt = std::make_unique<Ast::Return>(std::move(unary));
    auto fn = std::make_unique<Ast::Function>("main", std::move(stmt));
    auto program = Ast::AST(std::move(fn));
    Tacky::Generator gen;
    auto tacky_ir = gen.convert_ast(program);

    CHECK(tacky_ir->fn()->body().size() == 2);
    CHECK(dynamic_cast<Tacky::Unary *>(tacky_ir->fn()->body()[0].release()));
    CHECK(dynamic_cast<Tacky::Return *>(tacky_ir->fn()->body()[1].release()));
}

TEST_CASE("convert_function with one statement") {
    auto unary =
        std::make_unique<Ast::Unary>(std::make_unique<Ast::Complement>(),
                                     std::make_unique<Ast::Constant>("123"));
    auto stmt = std::make_unique<Ast::Return>(std::move(unary));
    auto fn = std::make_unique<Ast::Function>("main", std::move(stmt));
    Tacky::Generator gen;
    auto tacky_fn = gen.convert_function(fn);

    CHECK(tacky_fn->body().size() == 2);
    CHECK(tacky_fn->body()[0]->to_string() ==
          "Unary(Complement, Constant(123), Var(main.0))");
    CHECK(tacky_fn->body()[1]->to_string() == "Return(Var(main.0))");
}

TEST_CASE("convert_statement for returning a nested unary complement") {
    // Return(Unary(Negate,
    //              Unary(Complement,
    //                    Unary(Negate, Constant(97)))))
    auto unary1 = std::make_unique<Ast::Unary>(
        std::make_unique<Ast::Negate>(), std::make_unique<Ast::Constant>("97"));
    auto unary2 = std::make_unique<Ast::Unary>(
        std::make_unique<Ast::Complement>(), std::move(unary1));
    auto unary3 = std::make_unique<Ast::Unary>(std::make_unique<Ast::Negate>(),
                                               std::move(unary2));
    auto stmt = std::make_unique<Ast::Return>(std::move(unary3));
    Tacky::Generator gen;
    gen.convert_statement(std::move(stmt));

    CHECK(gen.instructions().size() == 4);
    CHECK(gen.instructions()[0]->to_string() ==
          "Unary(Negate, Constant(97), Var(main.0))");
    CHECK(gen.instructions()[1]->to_string() ==
          "Unary(Complement, Var(main.0), Var(main.1))");
    CHECK(gen.instructions()[2]->to_string() ==
          "Unary(Negate, Var(main.1), Var(main.2))");
    CHECK(gen.instructions()[3]->to_string() == "Return(Var(main.2))");
}

TEST_CASE("convert_statement for returning a single unary complement") {
    // Return(Unary(Complement, Constant(123)))
    auto unary =
        std::make_unique<Ast::Unary>(std::make_unique<Ast::Complement>(),
                                     std::make_unique<Ast::Constant>("123"));
    auto stmt = std::make_unique<Ast::Return>(std::move(unary));
    Tacky::Generator gen;
    gen.convert_statement(std::move(stmt));

    CHECK(gen.instructions().size() == 2);
    CHECK(gen.instructions()[0]->to_string() ==
          "Unary(Complement, Constant(123), Var(main.0))");
    CHECK(gen.instructions()[1]->to_string() == "Return(Var(main.0))");
}

TEST_CASE("convert_statement for returning a constant") {
    // Return(Constant(88))
    auto exp = std::make_unique<Ast::Constant>("88");
    auto stmt = std::make_unique<Ast::Return>(std::move(exp));
    Tacky::Generator gen;
    gen.convert_statement(std::move(stmt));
    CHECK(gen.instructions().size() == 1);
    CHECK(gen.instructions()[0]->to_string() == "Return(Constant(88))");
}

TEST_CASE("convert_exp for a unary negate exp") {
    auto op = std::make_unique<Ast::Negate>();
    auto exp = std::make_unique<Ast::Constant>("420");
    auto unary = std::make_unique<Ast::Unary>(std::move(op), std::move(exp));
    Tacky::Generator gen;
    auto dst = gen.convert_exp(std::move(unary));

    CHECK(dst->value() == "main.0");
    CHECK(gen.instructions().size() == 1);
    auto tacky_unary =
        dynamic_cast<Tacky::Unary *>(gen.instructions()[0].release());
    CHECK(tacky_unary);
    CHECK(dynamic_cast<Tacky::Negate *>(tacky_unary->op().release()));
}

TEST_CASE("convert_exp for a unary complement exp") {
    auto op = std::make_unique<Ast::Complement>();
    auto exp = std::make_unique<Ast::Constant>("420");
    auto unary = std::make_unique<Ast::Unary>(std::move(op), std::move(exp));
    Tacky::Generator gen;

    auto dst = gen.convert_exp(std::move(unary));
    CHECK(dst->value() == "main.0");
    CHECK(gen.instructions().size() == 1);
    auto tacky_unary =
        dynamic_cast<Tacky::Unary *>(gen.instructions()[0].release());
    CHECK(tacky_unary);
    CHECK(dynamic_cast<Tacky::Complement *>(tacky_unary->op().release()));
}

TEST_CASE("convert_exp for a constant") {
    Tacky::Generator gen;
    auto exp = std::make_unique<Ast::Constant>("42");
    auto dst = gen.convert_exp(std::move(exp));
    CHECK(dst->value() == "42");
    CHECK(gen.instructions().size() == 0);
}
