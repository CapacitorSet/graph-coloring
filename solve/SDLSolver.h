#ifndef GRAPH_COLORING_SDLSOLVER_H
#define GRAPH_COLORING_SDLSOLVER_H

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

class SDLSolver : public Solver {
    uint32_t num_threads;

    void apply_weighting_phase(const Graph &graph, std::vector<uint32_t> &degrees, std::vector<uint32_t> &weights, uint32_t from, uint32_t to);

    void apply_coloring_phase(const std::vector<uint32_t> &degrees, uint32_t vertex, uint32_t range, Graph &graph);

public:
    SDLSolver(int num_threads);

    std::string name() const;

    void solve(Graph &);
};

#endif //GRAPH_COLORING_SDLSOLVER_H
