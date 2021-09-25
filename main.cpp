#include "benchmark/Benchmark.h"
#include "parse/Parser.h"
#include <algorithm>
#include <iostream>

int main(int argc, char **argv) {
    std::vector<std::string> args(argv, argv + argc);
    if (args.size() < 2) {
        std::cerr << "Syntax: " << args[0] << " [--parse-only] [--csv] <graph>" << std::endl;
        return 1;
    }
    bool use_csv = std::find(args.begin(), args.end(), "--csv") != args.end();
    bool parse_only = std::find(args.begin(), args.end(), "--parse-only") != args.end();

    Parser p(args.back());
    Graph graph = p.parse();
    if (use_csv) {
        if (parse_only) {
            std::cout << p.metadata.filename << ";" << std::to_string(p.metadata.num_vertices) << ";"
                      << std::to_string(p.metadata.num_edges) << ";";
            std::cout << std::to_string(long(p.milliseconds)) << std::endl;
        }
    } else {
        std::cout << "Parsed in " << std::to_string(long(p.milliseconds)) << " ms." << std::endl;
    }
    if (parse_only)
        return 0;

    Benchmark bench(graph);
    if (use_csv) {
        bench.settings.output = bench.settings.USE_CSV;
        bench.settings.parse_md = &p.metadata;
    }
    bench.run();
    return 0;
}
