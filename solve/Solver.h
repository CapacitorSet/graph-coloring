#ifndef GRAPH_COLORING_SOLVER_H
#define GRAPH_COLORING_SOLVER_H

#include "../graph/Graph.h"
#include <string>

#if WITH_RANDOM_SEED
#define RANDOM_SEED (std::random_device()())
#else
#define RANDOM_SEED 123
#endif

// Define a common solver interface for benchmarking.

class Solver {
  public:
    // A virtual dtor allows us to `delete` pointers to Solver (eg. in Benchmark::solvers).
    virtual ~Solver(){};

    virtual std::string name() const = 0;

    virtual void solve(Graph &) = 0;
};

#endif //GRAPH_COLORING_SOLVER_H
