#ifndef GRAPH_COLORING_LDFSOLVER_H
#define GRAPH_COLORING_LDFSOLVER_H

#include "Solver.h"

class LDFSolver : public Solver {
    uint32_t num_threads;
    
    void coloring_in_parallel(std::vector<uint32_t> &degrees, uint32_t vertex, uint32_t range, Graph &original_graph);

public:
    LDFSolver(int num_threads);

    std::string name() const;

    void solve(Graph&);

};


#endif //GRAPH_COLORING_LDFSOLVERPARALLEL_H
