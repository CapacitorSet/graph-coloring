#include <numeric>
#include <algorithm>
#include <random>
#include <set>
#include "SequentialSolver.h"

void SequentialSolver::solve(Graph &graph) {
    std::random_device random_dev;
    std::mt19937 random_gen(random_dev());

    // Create a random permutation of {v_0, v_1... v_n}
    std::vector<uint32_t> permutation(graph.vertices.size());
    // std::iota begins from 1, so begin() + 1 ensures that we start from 0
    std::iota(permutation.begin() + 1, permutation.end(), 1);
    std::shuffle(permutation.begin(), permutation.end(), random_gen);

    // For each element...
    for (uint32_t index : permutation) {
        std::set<color_t> neighbor_colors;
        for (auto neighbor : graph.neighbors_of(index))
            neighbor_colors.emplace(graph.color_of(neighbor));

        // Find smallest color not in the set of neighbor colors
        color_t smallest_color = 0;
        for (uint32_t neighbor_color : neighbor_colors)
            if (smallest_color != neighbor_color)
                break;
            else
                smallest_color++;

        graph.colors[index] = smallest_color;
    }
}

SequentialSolver::SequentialSolver() {
    this->name = "SequentialSolver";
}
