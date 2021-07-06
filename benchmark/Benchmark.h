#ifndef GRAPH_COLORING_BENCHMARK_H
#define GRAPH_COLORING_BENCHMARK_H

#include "../solve/SequentialSolver.h"
#include "../solve/LubySolver.h"

struct result {
    bool success;
    uint32_t num_colors;
    double milliseconds;
};

class Benchmark {
    SequentialSolver sequential;
    LubySolver luby;

    Graph graph;

    // Return the number of milliseconds when using the given solver
    struct result run_single(Solver *);

public:
    Benchmark(Graph &);

    void run();
};


#endif //GRAPH_COLORING_BENCHMARK_H
