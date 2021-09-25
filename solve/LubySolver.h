#ifndef GRAPH_COLORING_LUBYSOLVER_H
#define GRAPH_COLORING_LUBYSOLVER_H

#include "../utils/RangeSplitter.h"
#include "Solver.h"
#include <random>
#include <set>
#include <thread>

class LubySolver : public Solver {
    int num_threads;
    std::mt19937 gen;
    std::vector<std::thread> threads;
    std::vector<uint32_t> MIS;

    // Variables for probabilistic_select
    std::vector<uint32_t> S;
    std::vector<char> S_bitmap;
    std::vector<std::vector<uint32_t>> partial_S;
    std::set<uint32_t> V;
    // Mirrors V into a vector so that each thread can work on part of it using V_splitter
    std::vector<uint32_t> V_vec;
    VectorSplitter<uint32_t> *V_splitter;

    bool kill_threads;
    pthread_barrier_t thread_start_barrier, thread_end_barrier;

    // First step
    inline void probabilistic_select(const Graph &);
    // Second step
    inline void remove_edges(const Graph &);
    // Luby's algorithm
    void compute_MIS(const DeletableGraph &src);

  public:
    LubySolver(int num_threads = 1);
    ~LubySolver();

    std::string name() const;

    void solve(Graph &);
};

#endif //GRAPH_COLORING_LUBYSOLVER_H
