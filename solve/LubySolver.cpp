#include "LubySolver.h"
#include <algorithm>
#include <random>
#include <set>

LubySolver::LubySolver() {
    this->name = "LubySolver";
}

void LubySolver::solve(Graph &original_graph) {
    Graph graph(original_graph);
    color_t color = 0;
    while (!graph.empty()) {
        std::vector<uint32_t> MIS = maximal_independent_set(graph);
        for (uint32_t vertex : MIS) {
            original_graph.colors[vertex] = color;
            graph.remove_vertex(vertex);
        }
        color++;
    }
}

// Best explained here:
// https://en.wikipedia.org/wiki/Maximal_independent_set#Random-selection_parallel_algorithm_[Luby's_Algorithm]
std::vector<uint32_t> LubySolver::maximal_independent_set(const Graph &src) {
    std::mt19937 gen(RANDOM_SEED);

    std::vector<uint32_t> I;
    std::set<uint32_t> V;
    for (uint32_t i = 0; i < src.vertices.size(); i++)
        // It suffices to check for is_deleted here, since we don't delete vertices inside the function
        if (!src.is_deleted(i))
            V.emplace(i);

    // Check if empty _wrt src.deleted_!
    while (!V.empty()) {
        // The subset of vertices selected
        // We use std::vector as the iterator for std::set is very slow
        std::vector<uint32_t> S;
        // For each vertex, include it or not with probability 1/(2/d(v))
        for (uint32_t i : V) {
            double probability = 1. / (2 * src.degree_of(i));
            std::bernoulli_distribution d(probability);
            if (d(gen))
                S.emplace_back(i);
        }
        // Optimization: do not check for edges if we have less than 2 nodes
        if (S.size() >= 2) {
            // For each edge in E, check that "from" and "to" are in the graph.
            // Because src.vertices is sorted, and the vector of edges is also sorted in the Metis parser, we can use a
            // more efficient algo: set_intersection.

            // First, check that the "from" index is in the graph S.
            std::vector<uint32_t> from_candidates, from_indices(src.vertices.size());
            from_candidates.reserve(std::max(S.size(), src.vertices.size()));
            for (uint32_t i = 0; i < src.vertices.size(); i++)
                from_indices[i] = i;

            std::set_intersection(
                    from_indices.cbegin(), from_indices.cend(),
                    S.cbegin(), S.cend(),
                    std::back_inserter(from_candidates));

            for (uint32_t from : from_candidates) {
                // Note that we do not need to check for is_deleted.
                // If an element is in S it was in V, which we already checked.

                // Then, check that the "to" index is in the graph S.
                const edges_t &neighbors = src.vertices[from];
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
                    if (src.degree_of(from) <= src.degree_of(to)) {
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

        for (uint32_t v : S) {
            I.emplace_back(v);
            V.erase(v);
            for (uint32_t neighbor : src.neighbors_of(v))
                V.erase(neighbor);
        }
    }

    return I;
}
