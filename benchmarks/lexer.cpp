#include <benchmark/benchmark.h>

#define DOCTEST_CONFIG_IMPLEMENT
#include "../src/doctest.h"
#include "../src/lexer.h"
#include "../src/parser.h"

const auto code = "int main(void) { return ~(-2); }";

static void Lexer_generate_tokens(benchmark::State &state) {
    std::stringstream source(code);
    for (auto _ : state) {
        tokenize(source);
    }
}
BENCHMARK(Lexer_generate_tokens);

BENCHMARK_MAIN();
