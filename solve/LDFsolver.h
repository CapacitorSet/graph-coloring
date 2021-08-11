#ifndef GRAPH_COLORING_LDFSOLVER_H
#define GRAPH_COLORING_LDFSOLVER_H

#include <random>
#include <set>
#include <iostream>
#include <algorithm>
#include <random>
#include <vector>
#include "Solver.h"

class LDFsolver : public Solver {
    int num_threads;

public:
    LDFsolver(int num_threads = 1);

    std::string name() const;

    void solve(Graph&);

};


#endif //GRAPH_COLORING_LDFSOLVER_H
