#ifndef GRAPH_COLORING_SDLSOLVERPARALLEL_H
#define GRAPH_COLORING_SDLSOLVERPARALLEL_H

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

class SDLsolverParallel : public Solver {
    uint32_t num_threads;

    std::mutex m;
    std::mutex wrt_mutex;
    std::condition_variable cv;
    pthread_barrier_t barrier1;
    pthread_barrier_t barrier2;

    void weighting_phase(std::vector<uint32_t> &degrees, std::vector<uint32_t> &weights, Graph &original_graph, uint32_t &num_vertices);
    void coloring_phase(std::vector<uint32_t> &weights, Graph &original_graph);

public:
    SDLsolverParallel(int num_threads);

    std::string name() const;

    void solve(Graph&);

};


#endif //GRAPH_COLORING_SDLSOLVERPARALLEL_H
