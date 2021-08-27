#include <chrono>
#include <iostream>
#include "Benchmark.h"
#include "MemoryMonitor.h"
#include "../solve/SequentialSolver.h"
#include "../solve/LubySolver.h"
#include "../solve/JonesSolver.h"
#include "../solve/RandomPrioritySolver.h"
#include "../solve/LDFsolver.h"
#include "../solve/LDFsolverParallel.h"

Benchmark::Benchmark(Graph &g) : solvers({
    new SequentialSolver(),
    new LubySolver(),
    new JonesSolver(1),
    new JonesSolver(2),
    new JonesSolver(4),
    new RandomPrioritySolver(),
    new LDFsolverParallel(1),
    new LDFsolver(),
}), graph(g) {}

void Benchmark::run() {
    for (Solver *s : solvers) {
        std::cout << s->name() << ":" << std::endl;
        struct result res = run_single(s);
        printf("%.2f ms, %.2f MB (%s, %d colors)\n\n",
               res.milliseconds,
               double(res.peak_mem_usage) / 1024 / 1024,
               res.success ? "success" : "fail",
               res.num_colors);
        delete s;
    }
}

struct result Benchmark::run_single(Solver *solver) {
    MemoryMonitor monitor;

    auto t1 = std::chrono::high_resolution_clock::now();
    solver->solve(graph);
    auto t2 = std::chrono::high_resolution_clock::now();
    monitor.stop();

    bool success = graph.is_well_colored();
    uint32_t num_colors = graph.count_colors();
    double milliseconds = std::chrono::duration<double, std::milli>(t2 - t1).count();
    // Subtract the memory usage from other data structures in the program
    uint64_t mem_usage = monitor.delta();
    graph.clear();
    return {success, num_colors, milliseconds, mem_usage};
}
