#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#define DOCTEST_CONFIG_IMPLEMENT
#include "codegen/emission.h"
#include "doctest.h"
#include "lexer.h"
#include "parser.h"
#include "tacky.h"

enum class Stage { Lex, Parse, Tacky, Codegen };

void compile(Stage stage, std::string filename) {
    std::ifstream file(filename);
    if (file) {
        auto tokens = tokenize(file);
        if (stage == Stage::Lex) {
            for (auto token : tokens) {
                std::cout << "Token: " << token.to_str() << std::endl;
            }
            return;
        }

        Ast::Parser parser(tokens);
        auto ast = parser.parse();
        if (stage == Stage::Parse) {
            std::cout << ast.to_string() << std::endl;
            return;
        }

        Tacky::Generator gen;
        auto tacky_ir = gen.convert_ast(ast);
        if (stage == Stage::Tacky) {
            std::cout << tacky_ir.to_string() << std::endl;
            return;
        }

        Asm::Generator asm_gen;
        auto assembly = asm_gen.generate_assembly(tacky_ir);
        Asm::emit_code(assembly, filename);
    }
}

int main(int argc, char *argv[]) {
    doctest::Context ctx;
    ctx.applyCommandLine(argc, argv);
    int test_results = ctx.run();
    if (ctx.shouldExit()) {
        return test_results;
    }

    compile(Stage(std::stoi(argv[2])), argv[1]);
    return test_results;
}
