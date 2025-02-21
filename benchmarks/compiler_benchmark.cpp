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

static void Parser_parse_tokens(benchmark::State &state) {
    std::stringstream source(code);
    Ast::Parser parser(tokenize(source));
    for (auto _ : state) {
        parser.parse();
    }
}

BENCHMARK(Lexer_generate_tokens);
BENCHMARK(Parser_parse_tokens);
BENCHMARK_MAIN();
