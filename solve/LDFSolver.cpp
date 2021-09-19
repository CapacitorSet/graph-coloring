#include "LDFSolver.h"
#include "../utils/RangeSplitter.h"
#include <numeric>
#include <thread>

LDFSolver::LDFSolver(int num_threads) : num_threads(num_threads) {}

void LDFSolver::solve(Graph &graph) {
    /* if a number of threads larger than the ability of the system, generate error */
    if (num_threads > std::thread::hardware_concurrency())
        throw std::runtime_error("Hardware concurrency exceeded: please use at most " +
                                 std::to_string(std::thread::hardware_concurrency()) + " threads");

    /* Each thread has a vertex to start from and a range of vertices to work on */
    RangeSplitter rs(graph.vertices.size(), num_threads);

    std::vector<std::thread> threads;

    for (int thread_idx = 0; thread_idx < num_threads; thread_idx++)
        threads.emplace_back(std::thread([thread_idx, &graph, this, rs]() {
            uint32_t from = rs.get_min(thread_idx), to = rs.get_max(thread_idx);
            /* Applying the coloring phase where coloring is done in order according to the assigned weights */
            coloring_in_parallel(from, to, graph);
        }));

    for (auto &th : threads) {
        th.join();
    }

    if(!wrong_ones.empty()) {
        for(const auto &vertex : wrong_ones) {
            graph.color_with_smallest(vertex);
        }
    }
}

void LDFSolver::coloring_in_parallel(uint32_t from, uint32_t to, Graph &graph) {
    /* Create a vector to represent the vertices to be colored in order and initialize it in ascending order */
    std::vector<uint32_t> vertices_to_color(to - from);
    std::iota(vertices_to_color.begin(), vertices_to_color.end(), from);

    /* Sort the vector by degree */
    std::stable_sort(vertices_to_color.begin(), vertices_to_color.end(), [&](int i, int j) {
        return graph.degree_of(i + from) > graph.degree_of(j + from);
    });

    /* start coloring according to the order assigned above where no two neighbors have the same color */
    for (const auto &vertex_to_color : vertices_to_color) {
        if(uint32_t my_color = graph.color_with_smallest(vertex_to_color)){
            for (const auto &neighbor : graph.vertices[vertex_to_color]) {
                if(my_color == graph.colors[neighbor]) {
                    wrong_ones.emplace_back(vertex_to_color);
                }
            }
        }
    }
}

std::string LDFSolver::name() const {
    return "LDFSolver (" + std::to_string(num_threads) + " threads)";
}
