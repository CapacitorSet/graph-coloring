#include "SDLSolver.h"

SDLSolver::SDLSolver(int num_threads) : num_threads(num_threads) {}

void SDLSolver::solve(Graph &graph) {
    
    uint32_t num_vertices = graph.vertices.size();

    std::vector<uint32_t> degrees(num_vertices, 0);
    std::vector<uint32_t> weights(num_vertices, 0);

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

    std::vector<std::thread> thread_Pool;

    while (thread_counter) {
        if (thread_counter == 1) {
            range = vertices_per_thread + remaining_vertices;
        }

        thread_Pool.emplace_back(std::thread([vertex, &range, &graph, &weights, &degrees, this]() {

            /* Applying the weighting phase where each vertex takes a weight according to a particular algorithm */
            apply_weighting_phase(graph, degrees, weights, vertex, (range + vertex));

            /* Applying the coloring phase where coloring is done in order according to the assigned weights */
            apply_coloring_phase(degrees, vertex, range, graph);
        }));
        vertex += vertices_per_thread;
        thread_counter--;
    }

    for (auto &th : thread_Pool) {
        th.join();
    }
}

void SDLSolver::apply_weighting_phase(Graph &original_graph, std::vector<uint32_t> &degrees, std::vector<uint32_t> &weights, const uint32_t vertex, const uint32_t &upper_bound) {

    /*Each vertex computes its degree and announcing it by saving it in a shared vector */
    for (uint32_t vertexID = vertex; vertexID < upper_bound; vertexID++) {
        degrees[vertexID] = original_graph.degree_of(vertexID);
    }

    /* Compute the maximum degree */
    uint32_t max_degree = 0;
    for (uint32_t vertexID = vertex; vertexID < upper_bound; vertexID++) {
        if (degrees[vertexID] > max_degree) {
            max_degree = degrees[vertexID];
        }
    }

    /* The weight and the degree the threads are dealing with. They are initialized to zero at the beginning */
    uint32_t CurrentWeight = 0;
    uint32_t CurrentDegree = 0;

    edges_t neighbors;
    /* Keep working till the globalDegree reached the max degree */
    while (CurrentDegree <= max_degree) {
        for (uint32_t vertexID = vertex; vertexID < upper_bound; vertexID++)  {
            if (degrees[vertexID] <= CurrentDegree && degrees[vertexID] > 0) {
                weights[vertexID] = CurrentWeight;
                degrees[vertexID] = 0;
                neighbors = original_graph.neighbors_of(vertexID);
                for (uint32_t neighbor : neighbors) {
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

void SDLSolver::apply_coloring_phase(std::vector<uint32_t> &weights, uint32_t vertex, const uint32_t range, Graph &original_graph) {

    /* Create a vector to represent the vertices to be colored in order and intialize it in ascending order */
    std::vector<uint32_t> vertices_to_color(range);
    std::iota(vertices_to_color.begin(), vertices_to_color.end(), vertex);

    /* Reorder the vactor according to be descendingly from the highest weight to the lowest weight */
    std::stable_sort(vertices_to_color.begin(), vertices_to_color.end(), [&](int i, int j) {
        return weights[i + vertex] > weights[j + vertex];
    });

    /* start coloring according to the order assigned above where no two neighbors have the same color */
    for (uint32_t vertex_to_color : vertices_to_color) {
        original_graph.color_with_smallest(vertex_to_color);
    }
}

std::string SDLSolver::name() const {
    return "SDLSolver (" + std::to_string(num_threads) + " threads)";
}
