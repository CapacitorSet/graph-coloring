#include "RandomSelectionSolver.h"
#include "../utils/RangeSplitter.h"
#include <numeric>
#include <thread>

RandomSelectionSolver::RandomSelectionSolver(int num_threads) : num_threads(num_threads), random_gen(RANDOM_SEED) {}

void RandomSelectionSolver::solve(Graph &graph) {
    /* if a number of threads larger than the ability of the system, generate error */
    if (num_threads > std::thread::hardware_concurrency())
        throw std::runtime_error("Hardware concurrency exceeded: please use at most " +
                                 std::to_string(std::thread::hardware_concurrency()) + " threads");

    /* Each thread has a vertex to start from and a range of vertices to work on */
    RangeSplitter rs(graph.num_vertices(), num_threads);

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

    for (const auto &vertex : wrong_ones) {
        graph.color_with_smallest(vertex);
    }
}

void RandomSelectionSolver::coloring_in_parallel(uint32_t from, uint32_t to, Graph &graph) {
    std::vector<uint32_t> random_order(to - from);

    std::iota(random_order.begin(), random_order.end(), from);
    std::shuffle(random_order.begin(), random_order.end(), random_gen);

    /* start coloring according to the order assigned above where no two neighbors have the same color */
    for (const auto &vertex_to_color : random_order) {
        uint32_t my_color = graph.color_with_smallest(vertex_to_color);
        for (const auto &neighbor : graph.neighbors_of(vertex_to_color)) {
            if (my_color == graph.colors[neighbor]) {
                wrong_ones.emplace_back(vertex_to_color);
                break;
            }
        }
    }
}

std::string RandomSelectionSolver::name() const {
    return "RandomSelectionSolver (" + std::to_string(num_threads) + " threads)";
}
