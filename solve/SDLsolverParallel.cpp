#include "SDLsolverParallel.h"
#include <stdio.h>

SDLsolverParallel::SDLsolverParallel(int num_threads) : num_threads(num_threads) {}

void SDLsolverParallel::solve(Graph &graph) {
    uint32_t num_vertices = graph.vertices.size();
    std::vector<uint32_t> degrees(num_vertices, 0);
    std::vector<uint32_t> weights(num_vertices, 0);

    /* Applying the weighting phase where each vertex takes a weight according to a particular algorithm */
    weighting_phase(degrees, weights, graph, num_vertices);
    
    /* Applying the coloring phase where coloring is done in order according to the assigned weights */
    coloring_phase(weights, graph);
}

void SDLsolverParallel::weighting_phase(std::vector<uint32_t> &degrees, std::vector<uint32_t> &weights, Graph &original_graph, uint32_t &num_vertices) {
    std::vector<std::thread> thread_Pool;

    uint32_t vertices_per_thread = num_vertices / num_threads;
    uint32_t remaining_vertices = num_vertices % num_threads;

    if(num_threads > std::thread::hardware_concurrency()) {
        perror("Very large number of threads!! Please, use a smaller number of threads!\n");
        exit(0);
    }
    else if(num_threads > num_vertices) {
        perror("The entered number of threads is larger than number of vertices!! Please, use a number that is less than or equal the number of vertices\n");
        exit(0);
    }
    else {
        uint32_t vertex = 0;
        uint32_t range = vertices_per_thread;
        uint32_t thread_counter = num_threads;

        pthread_barrier_init(&barrier1, NULL, num_threads + 1);
        pthread_barrier_init(&barrier2, NULL, num_threads + 1);

        uint32_t counter = 0;
        uint32_t globalWeight = 0;
        uint32_t globalDegree = 0;
        uint32_t max_degree = 0;

        while(thread_counter) {

            if(thread_counter == 1){
                range = vertices_per_thread + remaining_vertices;
            }

            thread_Pool.emplace_back(std::thread([vertex, &range, &original_graph, &degrees, &weights, &counter, &globalDegree, &globalWeight, &max_degree, this]() {

                uint32_t vertexID = vertex;
                for(uint32_t i = 0; i < range; i++) {

                    // Each vertex computes its degree and announce it.
                    degrees[vertexID] = original_graph.degree_of(vertexID);
                    vertexID++;
                }

                /** Synchronization Points **/
                pthread_barrier_wait(&barrier1);
                pthread_barrier_wait(&barrier2);


                edges_t neighbors;
                
                while(globalDegree <= max_degree) {
                    std::unique_lock lock {m};
                    for(uint32_t i = 0; i < range; i++) {
                        if(degrees[vertexID] <= globalDegree && degrees[vertexID] > 0) {
                            weights[vertexID] = globalWeight;
                            degrees[vertexID] = 0;
                            neighbors = original_graph.neighbors_of(vertexID);
                            for(uint32_t neighbor : neighbors) {
                                if(degrees[neighbor] != globalDegree) {
                                degrees[neighbor]--;
                                }
                            }
                        }
                        vertexID++;
                    }

                    wrt_mutex.lock();
                    counter++;
                    wrt_mutex.unlock();

                    if(counter < num_threads) {
                        cv.wait(lock);
                    }
                    else {
                        counter = 0;
                        globalWeight++;
                        globalDegree++;
                        cv.notify_all();
                    }

                }
            }));

            vertex += vertices_per_thread;
            thread_counter--; 
        }

        /** Synchronization Points **/
        pthread_barrier_wait(&barrier1);
        for(uint32_t degree : degrees) {
            if(degree > max_degree) {
                max_degree = degree;
            }
        }
        pthread_barrier_wait(&barrier2);   


        for (auto &th : thread_Pool) {
            th.join();
        }

    }
}

void SDLsolverParallel::coloring_phase(std::vector<uint32_t> &weights, Graph &original_graph){

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

std::string SDLsolverParallel::name() const {
    return "SDLsolverParallel (" + std::to_string(num_threads) + " threads)";
}
