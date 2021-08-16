#ifndef GRAPH_COLORING_RANDOMPRIORITYSOLVER_H
#define GRAPH_COLORING_RANDOMPRIORITYSOLVER_H

#include <random>
#include <set>
#include <stdlib.h>
#include <barrier>
#include "Solver.h"
#include <mutex>

class RandomPrioritySolver : public Solver {
    int num_threads;
    std::vector<uint32_t> Random_Priority_Vec;
    std::vector<bool> Destroy_Vec;
    std::vector<uint32_t> MIS;
    std::vector<uint32_t> Remaining_Vertices;

    std::mutex wrt_mutex;

    void compute_MIS(const Graph &src);
    void vertex_job(uint32_t thID, Graph &src, std::barrier &sync_point1, std::barrier &sync_point2);
    
public:
    RandomPrioritySolver(int num_threads = 1);

    std::string name() const;

    void solve(Graph&);

};


#endif //GRAPH_COLORING_RANDOMPRIORITYSOLVER_H
