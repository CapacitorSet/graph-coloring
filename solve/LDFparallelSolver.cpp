#include "LDFparallelSolver.h"
#include <numeric>
#include <thread>

LDFparallelSolver::LDFparallelSolver(int num_threads) : num_threads(num_threads) {}

void LDFparallelSolver::solve(Graph &graph) {
    
    uint32_t num_vertices = graph.vertices.size();

    std::vector<uint32_t> degrees(num_vertices, 0);

    /* computing the number of vertices to be done per one thread */
    uint32_t vertices_per_thread = num_vertices / num_threads;
    uint32_t remaining_vertices = num_vertices % num_threads;

    /* if a number of threads larger than the ability of the system, generate error */
    if (num_threads > std::thread::hardware_concurrency())
        throw std::runtime_error("Hardware concurrency exceeded: please use at most " +
                                 std::to_string(std::thread::hardware_concurrency()) + " threads");

    // todo: fix
    if (num_threads > num_vertices)
        throw std::runtime_error("More threads than vertices: please use at most " +
                                 std::to_string(num_vertices) + " threads");

    /* Each thread has a vertex to start from and a range of vertices to work on */
    uint32_t vertex = 0;
    uint32_t range = vertices_per_thread;
    uint32_t thread_counter = num_threads;

    std::vector<std::thread> threads;

    while (thread_counter) {
        if (thread_counter == 1) {
            range = vertices_per_thread + remaining_vertices;
        }

        threads.emplace_back(std::thread([vertex, &range, &graph, &degrees, this]() {
            /* Applying the coloring phase where coloring is done in order according to the assigned weights */
            coloring_in_parallel(degrees, vertex, range, graph);
        }));
        vertex += vertices_per_thread;
        thread_counter--;
    }

    for (auto &th : threads) {
        th.join();
    }
}

void LDFparallelSolver::coloring_in_parallel(std::vector<uint32_t> &degrees, uint32_t vertex, uint32_t range, Graph &original_graph) {

    /* Create a vector to represent the vertices to be colored in order and intialize it in ascending order */
    std::vector<uint32_t> vertices_to_color(range);
    std::iota(vertices_to_color.begin(), vertices_to_color.end(), vertex);

    /* Reorder the vactor according to be descendingly from the highest weight to the lowest weight */
    std::stable_sort(vertices_to_color.begin(), vertices_to_color.end(), [&](int i, int j) {
        return original_graph.degree_of(i + vertex) > original_graph.degree_of(j + vertex);
    });

    /* start coloring according to the order assigned above where no two neighbors have the same color */
    for (uint32_t vertex_to_color : vertices_to_color) {
        original_graph.color_with_smallest(vertex_to_color);
    }
}

std::string LDFparallelSolver::name() const {
    return "LDFparallelSolver (" + std::to_string(num_threads) + " threads)";
}
