#include <numeric>
#include <algorithm>
#include <random>
#include <set>
#include "SequentialSolver.h"

void SequentialSolver::solve(Graph &graph) {
    std::mt19937 random_gen(RANDOM_SEED);

    // Create a random permutation of {v_0, v_1... v_n}
    std::vector<uint32_t> permutation(graph.vertices.size());
    // std::iota begins from 1, so begin() + 1 ensures that we start from 0
    std::iota(permutation.begin() + 1, permutation.end(), 1);
    std::shuffle(permutation.begin(), permutation.end(), random_gen);

    // For each vertex...
    for (uint32_t index : permutation)
        // Color it with the smallest color not in its neighbors' colors
        graph.color_with_smallest(index);
}

std::string SequentialSolver::name() const {
    return "SequentialSolver";
}
