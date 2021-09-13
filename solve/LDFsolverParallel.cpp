#include "LDFsolverParallel.h"
#include <stdio.h>

LDFsolverParallel::LDFsolverParallel(int num_threads) : num_threads(num_threads) {}

void LDFsolverParallel::solve(Graph &graph) {

    uint32_t num_vertices = graph.vertices.size();

    /* Initializing vector "degrees" to have number of elements equal to number of vertices */
    std::vector<uint32_t> degrees(num_vertices, 0);

    /* Computing the degree of each vertex */
    compute_degrees_in_parallel(graph, degrees, num_vertices);
    
    /* Creating a vector to represent the vertices to be colored in order */
    std::vector<uint32_t> vertices_to_color(degrees.size());

    /* Ordering vertices to be colored from highest order to the least order */
    order_vertices_to_be_colored(degrees, vertices_to_color);

    /* Coloring vertices according to the assigned order where no two neighbors have the same color */
    for (uint32_t vertex_to_color : vertices_to_color) {
        graph.color_with_smallest(vertex_to_color);
    }
}

void LDFsolverParallel::compute_degrees_in_parallel(Graph &original_graph, std::vector<uint32_t> &degrees, uint32_t num_vertices) {
    std::vector<std::thread> thread_Pool;

    /* computing the number of vertices to be done per one thread */
    uint32_t vertices_per_thread = num_vertices / num_threads;
    uint32_t remaining_vertices = num_vertices % num_threads;

    /* if a number of threads larger than the ability of the system, generate error */
    if(num_threads > std::thread::hardware_concurrency()) {
        perror("Very large number of threads!! Please, use a smaller number of threads!\n");
        exit(0);
    }
    /* if a number of threads greater than the number of vertices, generate error */
    else if(num_threads > num_vertices) {
        perror("The entered number of threads is larger than number of vertices!! Please, use a number that is less than or equal the number of vertices\n");
        exit(0);
    }
    else {
        /* Each thread has a vertex to start from and a range of vertices to work on */
        uint32_t vertex = 0;
        uint32_t range = vertices_per_thread;
        uint32_t thread_counter = num_threads;

        while(thread_counter) {

            /* If number of vertices to work on is not dividable by number of threads, then the last thread will do the remaining ones beside division */
            if(thread_counter == 1){
                range = vertices_per_thread + remaining_vertices;
            }

            thread_Pool.emplace_back(std::thread([vertex, &range, &original_graph, &degrees, this]() {

                /*Each vertex computes its degree and announcing it by saving it in a shared vector */
                uint32_t vertexTh = vertex;
                for(uint32_t i = 0; i < range; i++) {
                    degrees[vertexTh] = original_graph.degree_of(vertexTh);
                    vertexTh++;
                }
                
            }));

            vertex += vertices_per_thread;
            thread_counter--; 
        }
    }

    for (auto &th : thread_Pool) {
        th.join();
    }

}


void LDFsolverParallel::order_vertices_to_be_colored(std::vector<uint32_t> &degrees, std::vector<uint32_t> &vertices_to_color) {

    /* Intialize the vector in an ascending order */
    std::iota(vertices_to_color.begin(), vertices_to_color.end(), 0);

    /* Reorder the vactor according to be descendingly from the highest weight to the lowest weight */
    std::stable_sort(vertices_to_color.begin(), vertices_to_color.end(), [&](int i, int j) {
        return degrees[i] > degrees[j];
    });
}

std::string LDFsolverParallel::name() const {
    return "LDFsolverParallel (" + std::to_string(num_threads) + " threads)";
}

