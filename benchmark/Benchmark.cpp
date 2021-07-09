#include <chrono>
#include <iostream>
#include "Benchmark.h"
#include "../solve/SequentialSolver.h"
#include "../solve/LubySolver.h"

Benchmark::Benchmark(Graph &g) : solvers({
    new SequentialSolver(),
    new LubySolver(),
}), graph(g) {}

void Benchmark::run() {
    for (Solver *s : solvers) {
        std::cout << s->name() << ":" << std::endl;
        struct result res = run_single(s);
        printf("%.2f ms (%s, %d colors)\n\n",
               res.milliseconds,
               res.success ? "success" : "fail",
               res.num_colors);
    }
}

struct result Benchmark::run_single(Solver *solver) {
    auto t1 = std::chrono::high_resolution_clock::now();
    solver->solve(graph);
    auto t2 = std::chrono::high_resolution_clock::now();
    bool success = graph.is_well_colored();
    uint32_t num_colors = graph.count_colors();
    graph.clear_colors();

    double milliseconds = std::chrono::duration<double, std::milli>(t2 - t1).count();
    return {success, num_colors, milliseconds};
}
