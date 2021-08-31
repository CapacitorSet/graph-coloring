#include "LDFsolverParallel.h"


LDFsolverParallel::LDFsolverParallel(int num_threads) : num_threads(num_threads) {}


void LDFsolverParallel::solve(Graph &original_graph) {

    uint32_t num_vertices = original_graph.vertices.size();

    std::vector<uint32_t> degrees(num_vertices, 0);

    std::vector<std::thread> thread_Pool;


    for (uint32_t vertex = 0; vertex < num_vertices; vertex++) {

        thread_Pool.emplace_back(std::thread([&vertex, &original_graph, &degrees, this]() {

            // Each vertex computes its degree and announce it.
            m.lock();

            degrees[vertex] = original_graph.degree_of(vertex);

            m.unlock();

        }));
    }

    for (auto &th : thread_Pool) {
        th.join();
    }

    std::vector<uint32_t> vertices_to_color(degrees.size());

    compute_vertices_to_color_in_order(degrees, vertices_to_color);

    for (uint32_t vertex_to_color : vertices_to_color) {s
        original_graph.color_with_smallest(vertex_to_color);
    }
}


void LDFsolverParallel::compute_vertices_to_color_in_order(std::vector<uint32_t> &degrees, std::vector<uint32_t> &vertices_to_color) {

    std::iota(vertices_to_color.begin(), vertices_to_color.end(), 0);

    std::stable_sort(vertices_to_color.begin(), vertices_to_color.end(), [&](int i, int j) {

        return degrees[i] > degrees[j];

    });

}


std::string LDFsolverParallel::name() const {

    return "LDFsolverParallel (" + std::to_string(num_threads) + " threads)";

}

