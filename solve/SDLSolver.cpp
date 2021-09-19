#include "SDLSolver.h"
#include "../utils/RangeSplitter.h"

SDLSolver::SDLSolver(int num_threads) : num_threads(num_threads) {}

void SDLSolver::solve(Graph &graph) {

    uint32_t num_vertices = graph.vertices.size();

    std::vector<uint32_t> degrees(num_vertices, 0);
    std::vector<uint32_t> weights(num_vertices, 0);

    /* if a number of threads larger than the ability of the system, generate error */
    if (num_threads > std::thread::hardware_concurrency())
        throw std::runtime_error("Hardware concurrency exceeded: please use at most " + std::to_string(std::thread::hardware_concurrency()) + " threads");

    /* Each thread has a vertex to start from and a range of vertices to work on */
    RangeSplitter rs(num_vertices, num_threads);

    std::vector<std::thread> threads;

    for (int thread_idx = 0; thread_idx < num_threads; thread_idx++) {
        threads.emplace_back(std::thread([&graph, &weights, &degrees, this, rs, thread_idx]() {
            uint32_t from = rs.get_min(thread_idx), to = rs.get_max(thread_idx);

            /* Applying the weighting phase where each vertex takes a weight according to a particular algorithm */
            apply_weighting_phase(graph, degrees, weights, from, to);

            /* Applying the coloring phase where coloring is done in order according to the assigned weights */
            apply_coloring_phase(degrees, from, to, graph);
        }));
    }

    for (auto &th : threads) {
        th.join();
    }

    if(!wrong_ones.empty()) {
        for(const auto &vertex : wrong_ones) {
            graph.color_with_smallest(vertex);
        }
    }
}

void SDLSolver::apply_weighting_phase(const Graph &graph, std::vector<uint32_t> &degrees, std::vector<uint32_t> &weights, uint32_t from, uint32_t to) {

    /*Each from computes its degree and announcing it by saving it in a shared vector */
    for (uint32_t vertexID = from; vertexID < to; vertexID++) {
        degrees[vertexID] = graph.degree_of(vertexID);
    }

    /* Compute the maximum degree */
    uint32_t max_degree = 0;
    for (uint32_t vertexID = from; vertexID < to; vertexID++)
        max_degree = std::max(max_degree, degrees[vertexID]);

    /* The weight and the degree the threads are dealing with. They are initialized to zero at the beginning */
    uint32_t CurrentWeight = 0;
    uint32_t CurrentDegree = 0;

    /* Keep working till the globalDegree reached the max degree */
    while (CurrentDegree <= max_degree) {
        for (uint32_t vertexID = from; vertexID < to; vertexID++)  {
            if (degrees[vertexID] <= CurrentDegree && degrees[vertexID] > 0) {
                weights[vertexID] = CurrentWeight;
                degrees[vertexID] = 0;
                for (uint32_t neighbor : graph.neighbors_of(vertexID)) {
                    if (degrees[neighbor] != CurrentDegree) {
                        degrees[neighbor]--;
                    }
                }
            }
        }
        CurrentWeight++;
        CurrentDegree++;
    }
}

void SDLSolver::apply_coloring_phase(const std::vector<uint32_t> &weights, uint32_t from, uint32_t to, Graph &graph) {
    /* Create a vector to represent the vertices to be colored in order and initialize it in ascending order */
    std::vector<uint32_t> vertices_to_color(to - from);
    std::iota(vertices_to_color.begin(), vertices_to_color.end(), from);

    /* Sort the vector by weight */
    std::stable_sort(vertices_to_color.begin(), vertices_to_color.end(), [&](int i, int j) {
        return weights[i + from] > weights[j + from];
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

std::string SDLSolver::name() const {
    return "SDLSolver (" + std::to_string(num_threads) + " threads)";
}
