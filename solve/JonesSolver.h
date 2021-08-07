#ifndef GRAPH_COLORING_JONESSOLVER_H
#define GRAPH_COLORING_JONESSOLVER_H

#include <atomic>
#include <functional>
#include <map>
#include <random>
#include <thread>
#include "Solver.h"
#include "../utils/PCVector.h"

class JonesSolver : public Solver {
    int num_threads;
    std::mt19937 gen;
    std::vector<uint32_t> rho;
    // Associates each vertex with the number of neighbors it is "waiting on" (uncolored with higher rho)
    std::vector<std::atomic<int>> waitlist;

public:
    JonesSolver(int num_threads = 1);

    std::string name() const;

    void solve(Graph &);
};


#endif //GRAPH_COLORING_JONESSOLVER_H
