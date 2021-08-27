#include <algorithm>
#include <iostream>
#include "parse/Parser.h"
#include "benchmark/Benchmark.h"

int main(int argc, char **argv) {
    std::vector<std::string> args(argv, argv+argc);
    if (args.size() < 2) {
        std::cerr << "Syntax: " << args[0] << " [--parse-only] <graph>" << std::endl;
        return 1;
    }

    Parser p(args.back());
    Graph graph = p.parse();
    std::cout << "Parsed in " << std::to_string(long(p.milliseconds)) << " ms." << std::endl;
    if (std::find(args.begin(),  args.end(), "--parse-only") != args.end())
        return 0;

    Benchmark bench(graph);
    bench.run();
    return 0;
}
