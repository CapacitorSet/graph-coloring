#ifndef GRAPH_COLORING_SDLPARALLELSOLVER_H

#define GRAPH_COLORING_SDLPARALLELSOLVER_H

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

class SDLparallelSolver : public Solver {
    uint32_t num_threads;

    void apply_weighting_phase(Graph &original_graph, std::vector<uint32_t> &degrees, std::vector<uint32_t> &weights, const uint32_t vertex, const uint32_t &range);

    void apply_coloring_phase(std::vector<uint32_t> &degrees, const uint32_t vertex, const uint32_t range, Graph &original_graph);

public:
    SDLparallelSolver(int num_threads);

    std::string name() const;

    void solve(Graph &);
};

#endif //GRAPH_COLORING_LDFSOLVERPARALLEL_H
