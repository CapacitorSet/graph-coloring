#ifndef GRAPH_COLORING_LDFSOLVER_H
#define GRAPH_COLORING_LDFSOLVER_H

#include <random>
#include <set>
#include <iostream>
#include <algorithm>
#include <random>
#include <vector>
#include <numeric>
#include "Solver.h"

class LDFsolver : public Solver {
    int num_threads;

public:
    LDFsolver(int num_threads = 1);

    std::string name() const;

    void solve(Graph&);

    void compute_Degrees(std::vector<uint32_t> &degrees, uint32_t num_vertices, Graph &original_graph);

    void compute_vertices_to_color_in_order(std::vector<uint32_t> &degrees, std::vector<uint32_t> &vertices_to_color);

};


#endif //GRAPH_COLORING_LDFSOLVER_H
