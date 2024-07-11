#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"
#include "lexer.h"
#include "parser.h"

int main(int argc, char *argv[]) {
    doctest::Context ctx;
    ctx.applyCommandLine(argc, argv);
    int test_results = ctx.run();
    if (ctx.shouldExit()) {
        return test_results;
    }

    std::ifstream file(argv[1]);
    if (file) {
        Lexer lexer(file);
        auto tokens = lexer.generate_tokens();
        Parser parser(tokens);
        parser.parse();
    }

    return test_results;
}
