#include <iostream>
#include "parse/Parser.h"
#include "benchmark/Benchmark.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Syntax: " << argv[0] << " <graph>" << std::endl;
        return 1;
    }
    Parser p(argv[1]);
    Graph graph = p.parse();
    std::cout << "Parsed in " << std::to_string(long(p.milliseconds)) << " ms." << std::endl;
    Benchmark bench(graph);
    bench.run();
    return 0;
}
