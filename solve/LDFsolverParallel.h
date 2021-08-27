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
#include <mutex>
#include <numeric>
#include <condition_variable>

class LDFsolverParallel : public Solver {
    uint32_t num_threads;

    std::mutex m;
    
    void compute_vertices_to_color_in_order(std::vector<uint32_t> &degrees, std::vector<uint32_t> &vertices_to_color);

public:
    LDFsolverParallel(int num_threads);

    std::string name() const;

    void solve(Graph&);

};


#endif //GRAPH_COLORING_LDFSOLVERPARALLEL_H
