#include <chrono>
#include <iostream>
#include "Benchmark.h"
#include "MemoryMonitor.h"
#include "../solve/SequentialSolver.h"
#include "../solve/LubySolver.h"
#include "../solve/JonesSolver.h"
#include "../solve/LDFsequentialSolver.h"
#include "../solve/LDFparallelSolver.h"
#include "../solve/SDLparallelSolver.h"
#include "../solve/SDLsequentialSolver.h"

Benchmark::Benchmark(Graph &g) : solvers({
    new SequentialSolver(),
    new LubySolver(1),
    new LubySolver(2),
    new LubySolver(3),
    new JonesSolver(1),
    new JonesSolver(2),
    new JonesSolver(3),
    new SDLsequentialSolver(),
    new SDLparallelSolver(1),
    new SDLparallelSolver(2),
    new SDLparallelSolver(3),
    new LDFsequentialSolver(),
    new LDFparallelSolver(1),
    new LDFparallelSolver(2),
    new LDFparallelSolver(3),
}), graph(g) {}

void Benchmark::run() {
    // CSV header
    if (settings.output == settings.USE_CSV)
        printf("Solver;Time elapsed;Memory usage;Success;Colors\n");

    for (Solver *s : solvers) {
        if (settings.output == settings.USE_TEXT) {
            std::cout << s->name() << ":" << std::endl;
            struct result res = run_single(s);
            printf("%.2f ms, %.2f MB (%s, %d colors)\n\n",
                   res.milliseconds,
                   double(res.peak_mem_usage) / 1024 / 1024,
                   res.success ? "success" : "fail",
                   res.num_colors);
        } else if (settings.output == settings.USE_CSV || settings.output == settings.USE_CSV_COMPACT) {
            std::cout << s->name() << ";";
            struct result res = run_single(s);
            printf("%.2f;%.2f;%d;%d",
                   res.milliseconds,
                   double(res.peak_mem_usage) / 1024 / 1024,
                   res.success,
                   res.num_colors);
            if (settings.output == settings.USE_CSV)
                std::cout << std::endl;
            else
                std::cout << ";";
            std::cout.flush();
        }
        delete s;
    }
    if (settings.output == settings.USE_CSV_COMPACT)
        std::cout << std::endl;
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
