#ifndef GRAPH_COLORING_LUBYSOLVER_H
#define GRAPH_COLORING_LUBYSOLVER_H

#include <random>
#include <set>
#include "Solver.h"

class LubySolver : public Solver {
    int num_threads;
    std::mt19937 gen;
    std::vector<uint32_t> MIS;
    std::set<uint32_t> V;

    // First step
    inline std::vector<char> probabilistic_select(const Graph &);
    // Second step
    inline void remove_edges(std::vector<char> &S, const Graph &);
    // Luby's algorithm
    void compute_MIS(const Graph &src);

public:
    LubySolver(int num_threads = 1);

    std::string name() const;

    void solve(Graph&);

};


#endif //GRAPH_COLORING_LUBYSOLVER_H
