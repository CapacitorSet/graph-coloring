#include "LDFsequentialSolver.h"
#include <numeric>

LDFsequentialSolver::LDFsequentialSolver(int num_threads) : num_threads(num_threads) {}

void LDFsequentialSolver::solve(Graph &graph) {
    std::vector<uint32_t> vertices_to_color = compute_vertices_to_color_in_order(graph);

    for (uint32_t vertex_to_color : vertices_to_color) {
        graph.color_with_smallest(vertex_to_color);
    }
}

std::vector<uint32_t> LDFsequentialSolver::compute_vertices_to_color_in_order(const Graph &graph) {
    std::vector<uint32_t> vertices_to_color(graph.vertices.size());
    std::iota(vertices_to_color.begin(), vertices_to_color.end(), 0);
    std::stable_sort(vertices_to_color.begin(), vertices_to_color.end(), [&](int i, int j) {
        return graph.degree_of(i) > graph.degree_of(j);
    });
    return vertices_to_color;
}

std::string LDFsequentialSolver::name() const
{
    return "LDFsequentialSolver (" + std::to_string(num_threads) + " threads)";
}
