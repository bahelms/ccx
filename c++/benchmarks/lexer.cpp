#include <benchmark/benchmark.h>

#define DOCTEST_CONFIG_IMPLEMENT
#include "../doctest.h"
#include "../src/lexer.h"
#include "../src/parser.h"

const auto code = "int main(void) { return ~(-2); }";

static void Lexer_generate_tokens(benchmark::State &state) {
    std::stringstream source(code);
    Lexer lex(source);
    for (auto _ : state) {
        lex.generate_tokens();
    }
}
BENCHMARK(Lexer_generate_tokens);

BENCHMARK_MAIN();
