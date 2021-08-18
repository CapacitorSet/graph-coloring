#include "LDFsolverParallel.h"

LDFsolverParallel::LDFsolverParallel() {}

void LDFsolverParallel::solve(Graph &original_graph) {
    num_vertices = original_graph.vertices.size();
    degrees.reserve(num_vertices);
    std::vector<std::thread> threadPool;

    for (uint32_t vertex = 0; vertex < num_vertices; vertex++) {
        threadPool.emplace_back(vertex_job, vertex, original_graph);
    }

    for (auto &th : threadPool) {
        th.join();
    }

    std::vector<uint32_t> vertices_to_color(degrees.size());
    compute_vertices_to_color_in_order(degrees, vertices_to_color);

    for (uint32_t vertex_to_color : vertices_to_color) {
        original_graph.color_with_smallest(vertex_to_color);
    }
}

void LDFsolverParallel::vertex_job(uint32_t vertex, Graph &graph) {
    // Each vertex computes its degree
    uint32_t myDegree = graph.degree_of(vertex);
    // Announce it to the other vertices
    degrees[vertex] = myDegree;
}

void LDFsolverParallel::compute_vertices_to_color_in_order(std::vector<uint32_t> &degrees, std::vector<uint32_t> &vertices_to_color) {
    std::iota(vertices_to_color.begin(), vertices_to_color.end(), 0);
    std::stable_sort(vertices_to_color.begin(), vertices_to_color.end(), [&](int i, int j) {
        return degrees[i] > degrees[j];
    });
}

std::string LDFsolverParallel::name() const {
    return "LDFsolverParallel (" + std::to_string(num_vertices) + " threads)";
}
