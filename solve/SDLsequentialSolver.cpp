#include "SDLsequentialSolver.h"

#include <stdio.h>

SDLsequentialSolver::SDLsequentialSolver(int num_threads) : num_threads(num_threads) {}

void SDLsequentialSolver::solve(Graph &graph) {

    uint32_t num_vertices = graph.vertices.size();
    std::vector<uint32_t> degrees(num_vertices, 0);
    std::vector<uint32_t> weights(num_vertices, 0);

    /* Applying the weighting phase where each vertex takes a weight according to a particular algorithm */
    weighting_phase(degrees, weights, graph, num_vertices);

    /* Applying the coloring phase where coloring is done in order according to the assigned weights */
    coloring_phase(weights, graph);
}


void SDLsequentialSolver::weighting_phase(std::vector<uint32_t> &degrees, std::vector<uint32_t> &weights, Graph &original_graph, uint32_t &num_vertices) {

    /* counter of the number of threads finished the work */
    uint32_t counter = 0;

    /* The weight and the degree the threads are dealing with. They are initialized to zero at the beginning */
    uint32_t globalWeight = 0;
    uint32_t globalDegree = 0;

    /*Each vertex computes its degree and announcing it by saving it in a shared vector */
    for(uint32_t vertex = 0; vertex < num_vertices; vertex++) {
        degrees[vertex] = original_graph.degree_of(vertex);
    }

    /* Maximum degree to stop at*/
    uint32_t max_degree = 0;
    for(uint32_t degree : degrees) {
        if(degree > max_degree) {
            max_degree = degree;
        }
    }

    edges_t neighbors;

    /* Keep working till the globalDegree reached the max degree */ 
    while(globalDegree <= max_degree) {
        for(uint32_t vertex = 0; vertex < num_vertices; vertex++) {
            if(degrees[vertex] <= globalDegree && degrees[vertex] > 0) {
                weights[vertex] = globalWeight;
                degrees[vertex] = 0;
                neighbors = original_graph.neighbors_of(vertex);
                for(uint32_t neighbor : neighbors) {
                    if(degrees[neighbor] != globalDegree) {
                        degrees[neighbor]--;
                    }
                }
            }
        }
        globalWeight++;
        globalDegree++;
    }
}

void SDLsequentialSolver::coloring_phase(std::vector<uint32_t> &weights, Graph &original_graph){

   /* Create a vector to represent the vertices to be colored in order and intialize it in ascending order */
    std::vector<uint32_t> vertices_to_be_colored(weights.size());
    std::iota(vertices_to_be_colored.begin(), vertices_to_be_colored.end(), 0);

   /* Reorder the vactor according to be descendingly from the highest weight to the lowest weight */
    std::stable_sort(vertices_to_be_colored.begin(), vertices_to_be_colored.end(), [&](uint32_t i, uint32_t j) {
        return weights[i] > weights[j];
    });

    /* start coloring according to the order assigned above where no two neighbors have the same color */
    for (uint32_t vertex_to_be_colored : vertices_to_be_colored) {
        original_graph.color_with_smallest(vertex_to_be_colored);
    }
}

std::string SDLsequentialSolver::name() const {
    return "SDLsequentialSolver (" + std::to_string(num_threads) + " threads)";
}
