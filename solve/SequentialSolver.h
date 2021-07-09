#ifndef GRAPH_COLORING_SEQUENTIALSOLVER_H
#define GRAPH_COLORING_SEQUENTIALSOLVER_H

#include "Solver.h"

class SequentialSolver : public Solver {
public:
    SequentialSolver() = default;

    std::string name() const;

    void solve(Graph&);
};


#endif //GRAPH_COLORING_SEQUENTIALSOLVER_H
