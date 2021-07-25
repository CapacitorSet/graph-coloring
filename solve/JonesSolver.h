#ifndef GRAPH_COLORING_JONESSOLVER_H
#define GRAPH_COLORING_JONESSOLVER_H

#include <random>
#include <thread>
#include <condition_variable>
#include <unordered_map>
#include "Solver.h"
#include "../utils/PCVector.h"

class JonesSolver;

class ColoringProcess {
public:
    std::vector<uint32_t> waitlist;

    ColoringProcess(uint32_t vertex, const JonesSolver &, const Graph &);

    // "Received" a vertex coloring. Returns whether the waitlist is empty (= we can color the main node)
    bool receive(uint32_t v);
};

class JonesSolver : public Solver {
    std::vector<std::thread> threads;
    // For each thread, hold a "queue" (vector) of vertices that have been colored and have yet to be "received"
    std::vector<PCVector<uint32_t>> thread_queue;

    int num_threads;
    std::mt19937 gen;
    // Signaled after the first for loop, when we receive rho for each neighbor
    pthread_barrier_t rho_barrier;
    std::vector<uint32_t> rho;

    // It's not clear why this doesn't work with Graph&. To-do: investigate.
    static void thread_function(uint32_t thread_idx, Graph *, JonesSolver &);

    void notify_vertex_changed(uint32_t changed, const Graph &graph);

    friend class ColoringProcess;

    inline uint32_t thread_for(uint32_t vertex) const;

public:
    JonesSolver(int num_threads = 1);
    ~JonesSolver();

    std::string name() const;

    void solve(Graph &);
};


#endif //GRAPH_COLORING_JONESSOLVER_H
