#ifndef GRAPH_COLORING_BENCHMARK_H
#define GRAPH_COLORING_BENCHMARK_H

#include <future>
#include "../solve/Solver.h"

struct result {
    bool success;
    uint32_t num_colors;
    double milliseconds;
    uint64_t peak_mem_usage;
};

class Benchmark {
    std::vector<Solver *> solvers;

    Graph graph;

    // Return the number of milliseconds when using the given solver
    struct result run_single(Solver *);

    static void mem_monitor_thread_function(std::promise<uint64_t> &result, std::future<void> &stop);
    static uint64_t current_mem_usage();

public:
    Benchmark(Graph &);

    void run();
};


#endif //GRAPH_COLORING_BENCHMARK_H
