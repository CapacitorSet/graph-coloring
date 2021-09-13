#ifndef GRAPH_COLORING_SDLSEQUENTIALSOLVER_H

#define GRAPH_COLORING_SDLSEQUENTIALSOLVER_H



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
#include<condition_variable>
#include <pthread.h>

class SDLsequentialSolver : public Solver {
    uint32_t num_threads;

    void weighting_phase(std::vector<uint32_t> &degrees, std::vector<uint32_t> &weights, Graph &original_graph, uint32_t &num_vertices);
    void coloring_phase(std::vector<uint32_t> &weights, Graph &original_graph);

public:
    SDLsequentialSolver(int num_threads = 1);

    std::string name() const;

    void solve(Graph&);

};
#endif //GRAPH_COLORING_SDLSOLVERPARALLEL_H

