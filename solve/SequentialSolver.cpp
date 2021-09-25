#include <numeric>
#include "SequentialSolver.h"

SequentialSolver::SequentialSolver() : random_gen(RANDOM_SEED) {}

void SequentialSolver::solve(Graph &graph) {
    // Create a random permutation of {v_0, v_1... v_n}
    std::vector<uint32_t> permutation(graph.vertices.size());
    std::iota(permutation.begin(), permutation.end(), 0);
    std::shuffle(permutation.begin(), permutation.end(), random_gen);

    // For each vertex...
    for (uint32_t index : permutation)
        // Color it with the smallest color not in its neighbors' colors
        graph.color_with_smallest(index);
}

std::string SequentialSolver::name() const {
    return "SequentialSolver";
}
