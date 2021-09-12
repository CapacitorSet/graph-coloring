#ifndef GRAPH_COLORING_RANDOMPRIORITYSOLVER_H
#define GRAPH_COLORING_RANDOMPRIORITYSOLVER_H

#include <random>
#include <set>
#include <cstdlib>
#include "Solver.h"
#include <mutex>
#include <algorithm>
#include <thread>
#include <pthread.h>

class RandomPrioritySolver : public Solver {
    int num_threads;
    std::vector<float> Random_Priority_Vec;
    std::vector<bool> Destroy_Vec;
    std::vector<uint32_t> MIS;
    std::vector<uint32_t> Remaining_Vertices;

    std::mutex wrt_mutex;

    void compute_MIS(const Graph &src);

public:
    RandomPrioritySolver(int num_threads = 1);

    std::string name() const;

    void solve(Graph&);

};


#endif //GRAPH_COLORING_RANDOMPRIORITYSOLVER_H
