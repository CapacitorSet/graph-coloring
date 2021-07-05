#ifndef GRAPH_COLORING_SOLVER_H
#define GRAPH_COLORING_SOLVER_H

#include <string>
#include "../graph/Graph.h"

#if WITH_RANDOM_SEED
#define RANDOM_SEED (std::random_device()())
#else
#define RANDOM_SEED 123
#endif

// Define a common solver interface for benchmarking.

class Solver {
public:
    std::string name;

    virtual void solve(Graph &) = 0;
};

#endif //GRAPH_COLORING_SOLVER_H
