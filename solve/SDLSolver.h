#ifndef GRAPH_COLORING_SDLSOLVER_H

#define GRAPH_COLORING_SDLSOLVER_H

#include "Solver.h"

#include <iostream>

#include <algorithm>

#include <random>

#include <numeric>
#include <thread>
#include <vector>

class SDLSolver : public Solver {

    uint32_t num_threads;

    std::vector<uint32_t> wrong_ones;

    void apply_weighting_phase(const Graph &graph, std::vector<uint32_t> &degrees, std::vector<uint32_t> &weights, uint32_t from, uint32_t to);

    void apply_coloring_phase(const std::vector<uint32_t> &degrees, uint32_t vertex, uint32_t range, Graph &graph);

  public:
    SDLSolver(int num_threads);

    std::string name() const;

    void solve(Graph &);
};

#endif //GRAPH_COLORING_SDLSOLVER_H
