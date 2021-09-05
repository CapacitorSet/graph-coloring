#include "LDFsolverParallel.h"
#include <stdio.h>

LDFsolverParallel::LDFsolverParallel(int num_threads) : num_threads(num_threads) {}

void LDFsolverParallel::solve(Graph &original_graph) {
    uint32_t num_vertices = original_graph.vertices.size();
    std::vector<uint32_t> degrees(num_vertices, 0);
    std::vector<std::thread> thread_Pool;

    uint32_t vertices_per_thread = num_vertices / num_threads;
    uint32_t remaining_vertices = num_vertices % num_threads;

    if(num_threads > std::thread::hardware_concurrency()) {
        perror("Very large number of threads!! Please, use a smaller number of threads!\n");
    }
    else if(num_threads > num_vertices) {
        perror("The entered number of threads is larger than number of vertices!! Please, use a number that is less than or equal the number of vertices\n");
    }
    else {
        uint32_t vertex = 0;
        uint32_t range = vertices_per_thread;
        uint32_t thread_counter = num_threads;

        while(thread_counter) {

            if(thread_counter == 1){
                range = vertices_per_thread + remaining_vertices;
            }

            thread_Pool.emplace_back(std::thread([vertex, &range, &original_graph, &degrees, this]() {
                uint32_t vertexTh = vertex;
                for(int i = 0; i < range; i++) {
                    // Each vertex computes its degree and announce it.
                    degrees[vertexTh] = original_graph.degree_of(vertexTh);
                    vertexTh++;
                }
            }));

            vertex += vertices_per_thread;
            thread_counter--; 
        }
    }

    for (uint32_t vertex = 0; vertex < num_vertices; vertex++) {
        thread_Pool.emplace_back(std::thread([&vertex, &original_graph, &degrees, this]() {
            // Each vertex computes its degree and announce it.
            degrees[vertex] = original_graph.degree_of(vertex);
        }));
    }

    for (auto &th : thread_Pool) {
        th.join();
    }

    std::vector<uint32_t> vertices_to_color(degrees.size());

    compute_vertices_to_color_in_order(degrees, vertices_to_color);

    for (uint32_t vertex_to_color : vertices_to_color) {
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

