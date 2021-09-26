#ifndef GRAPH_COLORING_RANDOMSELECTIONSOLVER_H
#define GRAPH_COLORING_RANDOMSELECTIONSOLVER_H

#include "Solver.h"
#include <random>

class RandomSelectionSolver : public Solver {
    uint32_t num_threads;
    std::mt19937 random_gen;

    std::vector<uint32_t> wrong_ones;

    void coloring_in_parallel(uint32_t from, uint32_t to, Graph &graph);

  public:
    RandomSelectionSolver(int num_threads);

    std::string name() const;

    void solve(Graph &);
};

#endif //GRAPH_COLORING_RANDOMSELECTIONSOLVER_H