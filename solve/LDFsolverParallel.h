#ifndef GRAPH_COLORING_LDFSOLVERPARALLEL_H
#define GRAPH_COLORING_LDFSOLVERPARALLEL_H

#include "Solver.h"
#include <random>
#include <set>
#include <iostream>
#include <algorithm>
#include <random>
#include <vector>
#include <thread>
#include <numeric>
#include <condition_variable>

class LDFsolverParallel : public Solver {
    std::vector<uint32_t> degrees;
    uint32_t num_vertices = 0;

    void vertex_job(uint32_t vertex, Graph &graph);
    void compute_vertices_to_color_in_order(std::vector<uint32_t> &degrees, std::vector<uint32_t> &vertices_to_color);

public:
    LDFsolverParallel();

    std::string name() const;

    void solve(Graph&);

};


#endif //GRAPH_COLORING_LDFSOLVERPARALLEL_H
