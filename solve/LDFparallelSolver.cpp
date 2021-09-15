#include "LDFparallelSolver.h"
#include <stdio.h>

LDFparallelSolver::LDFparallelSolver(int num_threads) : num_threads(num_threads) {}

void LDFparallelSolver::solve(Graph &graph) {
    
    uint32_t num_vertices = graph.vertices.size();

    std::vector<uint32_t> degrees(num_vertices, 0);
    std::vector<uint32_t> weights(num_vertices, 0);

    /* computing the number of vertices to be done per one thread */
    uint32_t vertices_per_thread = num_vertices / num_threads;
    uint32_t remaining_vertices = num_vertices % num_threads;

    /* if a number of threads larger than the ability of the system, generate error */
    if (num_threads > std::thread::hardware_concurrency()) {
        std::cout << "The largest number of threads can be used is: " << std::thread::hardware_concurrency() << std::endl;
        perror("Very large number of threads!! Please, use a smaller number !\n");
        return;
    }

    /* if a number of threads greater than the number of vertices, generate error */
    else if (num_threads > num_vertices) {
        perror("The entered number of threads is larger than number of vertices!! Please, use a number that is less than or equal the number of vertices\n");
        return;
    }

    else {

        /* Each thread has a vertex to start from and a range of vertices to work on */
        uint32_t vertex = 0;
        uint32_t range = vertices_per_thread;
        uint32_t thread_counter = num_threads;

        std::vector<std::thread> thread_Pool;

        while (thread_counter) {
            if (thread_counter == 1) {
                range = vertices_per_thread + remaining_vertices;
            }

            thread_Pool.emplace_back(std::thread([vertex, &range, &graph, &weights, &degrees, this]() {
                
                /* Applying the weighting phase where each vertex takes a weight according to a particular algorithm */
                compute_degrees(graph, degrees, vertex, (range + vertex));

                /* Applying the coloring phase where coloring is done in order according to the assigned weights */
                coloring_in_parallel(degrees, vertex, range, graph);
            }));
            vertex += vertices_per_thread;
            thread_counter--;
        }

        for (auto &th : thread_Pool) {
            th.join();
        }
    }
}

void LDFparallelSolver::compute_degrees(Graph &original_graph, std::vector<uint32_t> &degrees, const uint32_t vertex, const uint32_t &upper_bound) {

    /*Each vertex computes its degree and announcing it by saving it in a shared vector */
    for (uint32_t vertexID = vertex; vertexID < upper_bound; vertexID++) {
        degrees[vertexID] = original_graph.degree_of(vertexID);
    }
}

void LDFparallelSolver::coloring_in_parallel(std::vector<uint32_t> &degrees, uint32_t vertex, const uint32_t range, Graph &original_graph) {

    /* Create a vector to represent the vertices to be colored in order and intialize it in ascending order */
    std::vector<uint32_t> vertices_to_color(range);
    std::iota(vertices_to_color.begin(), vertices_to_color.end(), vertex);

    /* Reorder the vactor according to be descendingly from the highest weight to the lowest weight */
    std::stable_sort(vertices_to_color.begin(), vertices_to_color.end(), [&](int i, int j) {
        return degrees[i + vertex] > degrees[j + vertex];
    });

    /* start coloring according to the order assigned above where no two neighbors have the same color */
    for (uint32_t vertex_to_color : vertices_to_color) {
        original_graph.color_with_smallest(vertex_to_color);
    }
}

std::string LDFparallelSolver::name() const {
    return "LDFparallelSolver (" + std::to_string(num_threads) + " threads)";
}
