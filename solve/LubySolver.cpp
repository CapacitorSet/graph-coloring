#include "LubySolver.h"
#include <algorithm>
#include <random>
#include <thread>

LubySolver::LubySolver(int num_threads) : num_threads(num_threads), gen(RANDOM_SEED) {}

void LubySolver::solve(Graph &original_graph) {
    DeletableGraph uncolored_graph(original_graph);
    color_t color = 0;
    while (!uncolored_graph.empty()) {
        compute_MIS(uncolored_graph);
        for (uint32_t vertex : MIS) {
            original_graph.colors[vertex] = color;
            uncolored_graph.delete_vertex(vertex);
        }
        color++;
    }
}

// Best explained here:
// https://en.wikipedia.org/wiki/Maximal_independent_set#Random-selection_parallel_algorithm_[Luby's_Algorithm]
void LubySolver::compute_MIS(const DeletableGraph &del_graph) {
    // Reset solver state
    MIS.clear();
    V.clear();

    const Graph &graph = del_graph.graph;
    uint32_t num_vertices = graph.vertices.size();

    for (uint32_t i = 0; i < num_vertices; i++)
        // It suffices to check for is_deleted here, since we don't delete vertices inside the function
        if (!del_graph.is_deleted(i))
            V.emplace(i);

    while (!V.empty()) {
        // The subset of vertices selected
        // We use std::vector as the iterator for std::set is very slow
        probabilistic_select(graph);

        remove_edges(graph);

        for (uint32_t v : S) {
            if (!S_bitmap[v])
                continue;
            MIS.emplace_back(v);
            V.erase(v);
            for (uint32_t neighbor : graph.neighbors_of(v))
                V.erase(neighbor);
        }
    }
}

void LubySolver::probabilistic_select(const Graph &graph) {
    // Mirrors V into a vector so that each thread can work on part of it
    std::vector<uint32_t> V_vec(V.cbegin(), V.cend());
    S.clear();
    S_bitmap.clear();
    S_bitmap.resize(graph.vertices.size());
    int items_per_thread = ceil(float(V.size())/float(num_threads));
    std::vector<std::thread> threads;
    std::vector<std::vector<uint32_t>> partial_S(num_threads);
    for (int i = 0; i < num_threads; i++)
        threads.emplace_back([&](int thread_idx) {
            int V_start = items_per_thread * thread_idx,
                V_end = items_per_thread * (thread_idx+1);
            if (V_start >= V_vec.size())
                return;
            if (V_end >= V_vec.size())
                V_end = V_vec.size();
            auto &S = partial_S[thread_idx];
            int items_selected_here = 0;
            while (items_selected_here == 0) {
                // For each vertex, include it or not with probability 1/(2/d(v))
                for (int j = V_start; j < V_end; j++) {
                    auto vertex = V_vec[j];
                    double probability = 1. / (2 * graph.degree_of(vertex));
                    std::bernoulli_distribution d(probability);
                    if (d(gen)) {
                        S.push_back(vertex);
                        S_bitmap[vertex] = true;
                        items_selected_here++;
                    }
                }
            }
        }, i);
    for (int i = 0; i < num_threads; i++) {
        // Wait for each thread to finish processing its part of vertices
        threads[i].join();
        // Then concatenate the partial S
        S.insert(S.end(), partial_S[i].cbegin(), partial_S[i].cend());
    }
}

void LubySolver::remove_edges(const Graph &g) {
    // Optimization: do not check for edges if we have less than 2 nodes
    if (S.size() < 2)
        return;

    // For each edge in E, check that "from" and "to" are in the graph.

    // First, check that the "from" index is in S.
    for (uint32_t from : S) {
        // Then, check that the "to" index is also in S.
        /*
        // Equivalent to:
        for (uint32_t to : g.neighbors_of(from)) {
            if (to < from)
                continue;
        */
        auto &neighbors = g.neighbors_of(from);
        for (auto pos = std::lower_bound(neighbors.cbegin(), neighbors.cend(), from); pos != neighbors.cend(); ++pos) {
            uint32_t to = *pos;
            if (!S_bitmap[to])
                continue;
            if (g.degree_of(from) <= g.degree_of(to))
                S_bitmap[from] = false;
            else
                S_bitmap[to] = false;
        }
    }
}

std::string LubySolver::name() const {
    return "LubySolver (" + std::to_string(num_threads) + " threads)";
}
