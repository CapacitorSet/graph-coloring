#ifndef GRAPH_COLORING_LDFPARALLELSOLVER_H
#define GRAPH_COLORING_LDFPARELLELSOLVER_H

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

class LDFparallelSolver : public Solver {
    uint32_t num_threads;
    
    void compute_degrees(Graph &original_graph, std::vector<uint32_t> &degrees, const uint32_t vertex, const uint32_t &upper_bound);
    void coloring_in_parallel(std::vector<uint32_t> &degrees, uint32_t vertex, const uint32_t range, Graph &original_graph);

public:
    LDFparallelSolver(int num_threads);

    std::string name() const;

    void solve(Graph&);

};


#endif //GRAPH_COLORING_LDFSOLVERPARALLEL_H
