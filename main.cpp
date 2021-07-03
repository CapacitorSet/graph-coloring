#include <iostream>
#include "parse/Parser.h"
#include "benchmark/Benchmark.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Syntax: " << argv[0] << " <graph>" << std::endl;
        return 1;
    }
    Graph graph = Parser(argv[1]).parse();
    Benchmark bench(graph);
    bench.run();
    return 0;
}
