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

public:
    JonesSolver(int num_threads = 1);

    std::string name() const;

    void solve(Graph &);
};


#endif //GRAPH_COLORING_JONESSOLVER_H
