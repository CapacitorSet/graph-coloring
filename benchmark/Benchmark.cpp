#include <chrono>
#include <iostream>
#include "Benchmark.h"
#include "MemoryMonitor.h"
#include "../solve/SequentialSolver.h"
#include "../solve/LubySolver.h"
#include "../solve/JonesSolver.h"
#include "../solve/LDFSolver.h"
#include "../solve/SDLSolver.h"
#include "../solve/RandomSelectionSolver.h"

Benchmark::Benchmark(Graph &g) : solvers({
    new SequentialSolver(),
    new LubySolver(1),
    new LubySolver(2),
    new LubySolver(4),
    new JonesSolver(1),
    new JonesSolver(2),
    new JonesSolver(4),
    new SDLSolver(1),
    new SDLSolver(2),
    new SDLSolver(4),
    new LDFSolver(1),
    new LDFSolver(2),
    new LDFSolver(4),
    new RandomSelectionSolver(1),
    new RandomSelectionSolver(2),
    new RandomSelectionSolver(4),
}), graph(g) {}

void Benchmark::run() {
    // CSV header
    if (settings.output == settings.USE_CSV)
        printf("Graph;Vertices;Edges;Solver;Time elapsed;Memory usage;Success;Colors\n");

    for (Solver *s : solvers) {
        if (settings.output == settings.USE_TEXT) {
            std::cout << s->name() << ":" << std::endl;
            struct result res = run_single(s);
            printf("%.2f ms, %.2f MB (%s, %d colors)\n\n",
                   res.milliseconds,
                   double(res.peak_mem_usage) / 1024 / 1024,
                   res.success ? "success" : "fail",
                   res.num_colors);
        } else if (settings.output == settings.USE_CSV) {
            struct result res = run_single(s);
            printf("%s;%d;%d;%s;%.2f;%.2f;%d;%d\n",
                   settings.parse_md->filename.c_str(), settings.parse_md->num_vertices, settings.parse_md->num_edges,
                   s->name().c_str(),
                   res.milliseconds,
                   double(res.peak_mem_usage) / 1024 / 1024,
                   res.success,
                   res.num_colors);
            std::cout.flush();
        } else if (settings.output == settings.USE_CSV_COMPACT) {
            struct result res = run_single(s);
            printf("%s;%.2f;%.2f;%d;%d;",
                   s->name().c_str(),
                   res.milliseconds,
                   double(res.peak_mem_usage) / 1024 / 1024,
                   res.success,
                   res.num_colors);
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
