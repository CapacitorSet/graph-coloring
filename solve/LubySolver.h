#ifndef GRAPH_COLORING_LUBYSOLVER_H
#define GRAPH_COLORING_LUBYSOLVER_H

#include "Solver.h"

class LubySolver : public Solver {
    std::vector<uint32_t> maximal_independent_set(const Graph &src);

public:
    LubySolver();

    void solve(Graph&);

};


#endif //GRAPH_COLORING_LUBYSOLVER_H
