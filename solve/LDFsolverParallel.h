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
    
    void order_vertices_to_be_colored(std::vector<uint32_t> &degrees, std::vector<uint32_t> &vertices_to_color);
    void compute_degrees_in_parallel(Graph &original_graph, std::vector<uint32_t> &degrees, uint32_t num_vertices);

public:
    LDFsolverParallel(int num_threads);

    std::string name() const;

    void solve(Graph&);

};


#endif //GRAPH_COLORING_LDFSOLVERPARALLEL_H
