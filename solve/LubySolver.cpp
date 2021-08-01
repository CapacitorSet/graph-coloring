#include "LubySolver.h"
#include <algorithm>
#include <random>
#include <set>

LubySolver::LubySolver(int num_threads) : num_threads(num_threads), gen(RANDOM_SEED) {}

void LubySolver::solve(Graph &original_graph) {
    Graph uncolored_graph(original_graph);
    color_t color = 0;
    while (!uncolored_graph.empty()) {
        compute_MIS(uncolored_graph);
        for (uint32_t vertex : MIS) {
            original_graph.colors[vertex] = color;
            uncolored_graph.remove_vertex(vertex);
        }
        color++;
    }
}

// Best explained here:
// https://en.wikipedia.org/wiki/Maximal_independent_set#Random-selection_parallel_algorithm_[Luby's_Algorithm]
void LubySolver::compute_MIS(const Graph &src) {
    // Reset solver state
    MIS.clear();
    V.clear();

    for (uint32_t i = 0; i < src.vertices.size(); i++)
        // It suffices to check for is_deleted here, since we don't delete vertices inside the function
        if (!src.is_deleted(i))
            V.emplace(i);

    while (!V.empty()) {
        // The subset of vertices selected
        // We use std::vector as the iterator for std::set is very slow
        std::vector<uint32_t> S = probabilistic_select(src);

        remove_edges(S, src);

        for (uint32_t v : S) {
            MIS.emplace_back(v);
            V.erase(v);
            for (uint32_t neighbor : src.neighbors_of(v))
                V.erase(neighbor);
        }
    }
}

std::vector<uint32_t> LubySolver::probabilistic_select(const Graph &graph) {
    std::vector<uint32_t> S;
    // For each vertex, include it or not with probability 1/(2/d(v))
    for (uint32_t i : V) {
        double probability = 1. / (2 * graph.degree_of(i));
        std::bernoulli_distribution d(probability);
        if (d(gen))
            S.emplace_back(i);
    }
    return S;
}

void LubySolver::remove_edges(std::vector<uint32_t> &S, const Graph &g) {
    // Optimization: do not check for edges if we have less than 2 nodes
    if (S.size() < 2)
        return;

    // For each edge in E, check that "from" and "to" are in the graph.
    // Because g.vertices is sorted, and the vector of edges is also sorted in
    // the Metis parser, we can use a more efficient algo: set_intersection.

    // First, check that the "from" index is in the graph S.
    std::vector<uint32_t> from_candidates, from_indices(g.vertices.size());
    from_candidates.reserve(std::max(S.size(), g.vertices.size()));
    for (uint32_t i = 0; i < g.vertices.size(); i++)
        from_indices[i] = i;

    std::set_intersection(
            from_indices.cbegin(), from_indices.cend(),
            S.cbegin(), S.cend(),
            std::back_inserter(from_candidates));

    for (uint32_t from : from_candidates) {
        // Note that we do not need to check for is_deleted.
        // If an element is in S it was in V, which we already checked.

        // Then, check that the "to" index is in the graph S.
        const edges_t &neighbors = g.vertices[from];
        std::vector<uint32_t> edges_in_graph;
        edges_in_graph.reserve(std::max(S.size(), neighbors.size()));
        // If we already counted 1 -> 2, do not count 2 -> 1 again:
        // we start searching at the first neighbor higher than the "from" vertex
        auto first_new_neighbor = std::lower_bound(neighbors.cbegin(), neighbors.cend(), from);
        std::set_intersection(
                first_new_neighbor, neighbors.cend(),
                S.cbegin(), S.cend(),
                std::back_inserter(edges_in_graph));

        for (uint32_t to : edges_in_graph) {
            if (g.degree_of(from) <= g.degree_of(to)) {
                // std::lower_bound is std::find for sorted containers.
                auto from_pos = std::lower_bound(S.cbegin(), S.cend(), from);
                // The item may not be found if we already removed it
                if (from_pos != S.cend())
                    S.erase(from_pos);
            } else {
                auto to_pos = std::lower_bound(S.cbegin(), S.cend(), to);
                if (to_pos != S.cend())
                    S.erase(to_pos);
            }
        }
    }
}

std::string LubySolver::name() const {
    return "LubySolver (" + std::to_string(num_threads) + " threads)";
}
