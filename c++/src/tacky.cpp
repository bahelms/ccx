#include "tacky.h"
#include "../doctest.h"
#include "parser.h"
#include <memory>
#include <vector>

namespace Tacky {
std::unique_ptr<UnaryOperator>
convert_unop(std::unique_ptr<Ast::UnaryOperator> op) {
    if (auto *comp = dynamic_cast<Ast::Complement *>(op.get())) {
        return std::make_unique<Complement>();
    } else {
        return std::make_unique<Negate>();
    }
}

std::unique_ptr<Val>
convert_exp(std::unique_ptr<Ast::Exp> exp,
            std::vector<std::unique_ptr<Instruction>> &instrs) {
    if (auto *constant = dynamic_cast<Ast::Constant *>(exp.get())) {
        return std::make_unique<Constant>(constant->value());
    } else {
        auto unary(dynamic_cast<Ast::Unary *>(exp.get()));
        auto tacky_op = convert_unop(unary->op());
        auto src = convert_exp(unary->exp(), instrs);
        auto name = "main.1";
        auto instr = std::make_unique<Unary>(
            std::move(tacky_op), std::move(src), std::make_unique<Var>(name));
        instrs.emplace_back(std::move(instr));
        return std::make_unique<Var>(name);
    }
}
} // namespace Tacky

//// TESTS ////

TEST_CASE("convert_exp for a unary exp") {
    auto op = std::make_unique<Ast::Complement>();
    auto exp = std::make_unique<Ast::Constant>("420");
    auto unary = std::make_unique<Ast::Unary>(std::move(op), std::move(exp));
    auto instrs = std::vector<std::unique_ptr<Tacky::Instruction>>{};

    auto dst = Tacky::convert_exp(std::move(unary), instrs);
    CHECK(dst->value() == "main.1");
    CHECK(instrs.size() == 1);
    CHECK(dynamic_cast<Tacky::Unary *>(instrs[0].release()));
}

TEST_CASE("convert_exp for a constant") {
    auto exp = std::make_unique<Ast::Constant>("42");
    auto instrs = std::vector<std::unique_ptr<Tacky::Instruction>>{};
    auto dst = Tacky::convert_exp(std::move(exp), instrs);

    CHECK(dst->value() == "42");
    CHECK(instrs.size() == 0);
}
