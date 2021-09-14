#ifndef GRAPH_COLORING_LDFSEQUENTIALSOLVER_H
#define GRAPH_COLORING_LDFSEQUENTIALSOLVER_H

#include "Solver.h"

class LDFsequentialSolver : public Solver {
    int num_threads;

public:
    LDFsequentialSolver(int num_threads = 1);

    std::string name() const;

    void solve(Graph&);

    static std::vector<uint32_t> compute_vertices_to_color_in_order(const Graph &);

};


#endif //GRAPH_COLORING_LDFSOLVER_H
