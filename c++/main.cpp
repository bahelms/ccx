#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#define DOCTEST_CONFIG_IMPLEMENT
/* #include "codegen.h" */
#include "doctest.h"
#include "emission.h"
#include "lexer.h"
#include "parser.h"

enum class Stage { Lex, Parse, Codegen };

void compile(Stage stage, std::string filename) {
    std::ifstream file(filename);
    if (file) {
        Lexer lexer(file);
        auto tokens = lexer.generate_tokens();
        if (stage == Stage::Lex) {
            for (auto token : tokens) {
                std::cout << "Token: " << token.value() << std::endl;
            }
            return;
        }

        Parser parser(tokens);
        auto ast = parser.parse();
        if (stage == Stage::Parse) {
            std::cout << ast.to_string() << std::endl;
            return;
        }

        ASM::Codegen gen;
        auto program = gen.generate_program(ast);
        if (stage == Stage::Codegen) {
            return;
        }

        ASM::emit_code(program, filename);
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
