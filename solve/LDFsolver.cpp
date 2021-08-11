#include "LDFsolver.h"

LDFsolver::LDFsolver(int num_threads) : num_threads(num_threads) {}

void LDFsolver::solve(Graph &original_graph) {
    std::vector<uint32_t> degrees;
    uint32_t num_vertices = original_graph.vertices.size();

    for(uint32_t i = 0; i < num_vertices; i++) {
        degrees.emplace_back(original_graph.degree_of(i));
    }

    std::vector<uint32_t> vertices_to_color(degrees.size());
    std::iota(vertices_to_color.begin(),vertices_to_color.end(),0);
    std::stable_sort(vertices_to_color.begin(), vertices_to_color.end(), [&](int i,int j){return degrees[i]>degrees[j];});

    for(uint32_t vertex_to_color : vertices_to_color) {
        original_graph.color_with_smallest(vertex_to_color);
    }
}

std::string LDFsolver::name() const {
    return "LDFsolver (" + std::to_string(num_threads) + " threads)";
}
