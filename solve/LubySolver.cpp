#include "LubySolver.h"
#include <algorithm>
#include <random>

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
        std::vector<char> S = probabilistic_select(graph);

        remove_edges(S, graph);

        for (int v = 0; v < num_vertices; v++) {
            if (!S[v])
                continue;
            MIS.emplace_back(v);
            V.erase(v);
            for (uint32_t neighbor : graph.neighbors_of(v))
                V.erase(neighbor);
        }
    }
}

std::vector<char> LubySolver::probabilistic_select(const Graph &graph) {
    std::vector<char> S(graph.vertices.size());
    // For each vertex, include it or not with probability 1/(2/d(v))
    for (uint32_t i : V) {
        double probability = 1. / (2 * graph.degree_of(i));
        std::bernoulli_distribution d(probability);
        if (d(gen))
            S[i] = true;
    }
    return S;
}

void LubySolver::remove_edges(std::vector<char> &S, const Graph &g) {
    // Optimization: do not check for edges if we have less than 2 nodes
    if (S.size() < 2)
        return;

    // For each edge in E, check that "from" and "to" are in the graph.

    // First, check that the "from" index is in the graph S.
    uint32_t num_vertices = g.vertices.size();
    for (uint32_t from = 0; from < num_vertices; from++) {
        if (!S[from])
            continue;
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
            if (!S[to])
                continue;
            if (g.degree_of(from) <= g.degree_of(to))
                S[from] = false;
            else
                S[to] = false;
        }
    }
}

std::string LubySolver::name() const {
    return "LubySolver (" + std::to_string(num_threads) + " threads)";
}
