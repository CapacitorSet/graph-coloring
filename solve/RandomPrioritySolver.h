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
    uint32_t num_vertices_at_begining;
    std::mt19937 gen;
    std::vector<uint32_t> Random_Priority_Vec;
    std::vector<bool> Destroy_Vec;
    std::vector<uint32_t> MIS;
    std::vector<uint32_t> Remaining_Vertices;

    std::mutex wrt_mutex;

    void compute_MIS(const Graph &src);
    void vertex_job(uint32_t thID, Graph &src); //const std::vector<uint32_t> &Remaining_Vertices, std::vector<uint32_t> &Random_Priority_Vec, std::vector<uint32_t> &Destroy_Vec, std::vector<uint32_t> &MIS);

public:
    RandomPrioritySolver(int num_threads = 1, uint32_t num_vertices_at_begining);

    std::string name() const;

    void solve(Graph&);

};


#endif //GRAPH_COLORING_RANDOMPRIORITYSOLVER_H
