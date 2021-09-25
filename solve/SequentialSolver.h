#ifndef GRAPH_COLORING_SEQUENTIALSOLVER_H
#define GRAPH_COLORING_SEQUENTIALSOLVER_H

#include "Solver.h"
#include <random>

class SequentialSolver : public Solver {
    std::mt19937 random_gen;

public:
    SequentialSolver();

    std::string name() const;

    void solve(Graph&);
};


#endif //GRAPH_COLORING_SEQUENTIALSOLVER_H
