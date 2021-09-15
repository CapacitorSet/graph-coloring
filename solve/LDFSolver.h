#ifndef GRAPH_COLORING_LDFSOLVER_H
#define GRAPH_COLORING_LDFSOLVER_H

#include "Solver.h"

class LDFSolver : public Solver {
    uint32_t num_threads;
    
    void coloring_in_parallel(uint32_t from, uint32_t to, Graph &graph);

public:
    LDFSolver(int num_threads);

    std::string name() const;

    void solve(Graph&);

};


#endif //GRAPH_COLORING_LDFSOLVER_H
