#include <benchmark/benchmark.h>
#include "lexer.h"

static void BM_LexerSimpleMath(benchmark::State& state) {
    Lexer lexer;
    std::string code = "result = a + b * (c - d) / 2";

    for (auto _ : state) {
        auto ast = lexer.fromString(code);
        benchmark::DoNotOptimize(ast); // Чтобы компилятор не вырезал вызов
    }
}
BENCHMARK(BM_LexerSimpleMath);

static void BM_LexerComplexCode(benchmark::State& state) {
    Lexer lexer;
    std::string code =
        "def calculate_factorial(n):\n"
        "    if n <= 1:\n"
        "        return 1\n"
        "    else:\n"
        "        return n * calculate_factorial(n - 1)\n"
        "\n"
        "def main():\n"
        "    for i in range(100):\n"
        "        print(calculate_factorial(i))\n";

    for (auto _ : state) {
        auto ast = lexer.fromString(code);
        benchmark::DoNotOptimize(ast);
    }
}
BENCHMARK(BM_LexerComplexCode);

static void BM_FullCompilationCycle(benchmark::State& state) {
    Lexer lexer;
    std::string code =
        "while x < 1000:\n"
        "    x = x + 1\n"
        "    print(x)\n";

    std::vector<Compiler::TypeBinder> binders;

    for (auto _ : state) {
        auto ast = lexer.fromString(code);
        auto cppCode = Lexer::toCppString(ast, 0, &binders);
        benchmark::DoNotOptimize(cppCode);
    }
}
BENCHMARK(BM_FullCompilationCycle);

BENCHMARK_MAIN();
