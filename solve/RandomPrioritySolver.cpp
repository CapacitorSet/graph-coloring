#include "RandomPrioritySolver.h"

RandomPrioritySolver::RandomPrioritySolver(int num_threads) : num_threads(num_threads) {}

void RandomPrioritySolver::solve(Graph &original_graph) {
    Graph uncolored_graph(original_graph);
    color_t color = 0;

    srand((unsigned) time(NULL));

    while (!uncolored_graph.empty()) {
        compute_MIS(uncolored_graph);
        for (uint32_t vertex : MIS) {
            original_graph.colors[vertex] = color;
            uncolored_graph.remove_vertex(vertex);
        }
        color++;
    }
}

// Best explained here:
// https://en.wikipedia.org/wiki/Maximal_independent_set#Random-priority_parallel_algorithm

void RandomPrioritySolver::compute_MIS(const Graph &src) {
    // Reset solver state
    MIS.clear();
    Remaining_Vertices.clear();

    uint32_t num_verticies_graph = src.vertices.size();

    for (uint32_t i = 0; i < num_verticies_graph; i++) {
        // It suffices to check for is_deleted here, since we don't delete vertices inside the function
        if (!src.is_deleted(i)) {
            Remaining_Vertices.emplace_back(i);
        }
    }

    uint32_t num_vertices_Remaining;

    Random_Priority_Vec.reserve(num_verticies_graph);

    Destroy_Vec.assign(num_verticies_graph, false);

    while (!Remaining_Vertices.empty()) {
        std::vector<std::thread> threadPool;

        // Synchronization objects
        pthread_barrier_t   barrier1;
        pthread_barrier_t   barrier2;

        num_vertices_Remaining = Remaining_Vertices.size();

        uint32_t vertices_per_thread = num_vertices_Remaining / num_threads;
        uint32_t remaining_vertices = num_vertices_Remaining % num_threads;

        pthread_barrier_init(&barrier1, NULL, num_threads);
        pthread_barrier_init(&barrier2, NULL, num_threads);

        if(num_threads > std::thread::hardware_concurrency()) {
            perror("Very large number of threads!! Please, use a smaller number of threads!\n");
        }

        else if(num_threads > num_vertices_Remaining) {
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

                threadPool.emplace_back(std::thread([vertex, &range, &src, &barrier1, &barrier2, this] () {
                    uint32_t vertexID = vertex;

                    /** Each vertex selects a random number and announce it by saving it in a shared vector **/
                    for(uint32_t i = 0; i < range; i++) {
                        Random_Priority_Vec[vertexID] = rand() / RAND_MAX;
                        vertexID++;
                    }

                    /** Synchronization Point **/
                    pthread_barrier_wait(&barrier1);

                    vertexID = vertex;
                    edges_t neighbors;
                    edges_t Remaining_Neighbors;

                    for(uint32_t i = 0; i < range; i++) {

                        neighbors = src.neighbors_of(vertexID);
                        bool IamSmallest = true;

                        for(uint32_t neighbor : neighbors) {
                            if (std::find(Remaining_Vertices.begin(), Remaining_Vertices.end(), neighbor) != Remaining_Vertices.end()) {
                                Remaining_Neighbors.emplace_back(neighbor);
                                if(Random_Priority_Vec[neighbor] < Random_Priority_Vec[vertexID]) {
                                    IamSmallest = false;
                                    break;
                                }
                            }
                        }

                        if(IamSmallest) {

                            /** Insert itself in MIS **/
                            wrt_mutex.lock();
                            MIS.emplace_back(vertexID);
                            wrt_mutex.unlock();

                            /** Remove itself from Remaining_Verticies **/
                            std::remove(Remaining_Vertices.begin(), Remaining_Vertices.end(), vertexID);

                            /**  Inform Neighbors about Removal **/
                            for(uint32_t remaining_neighbor : Remaining_Neighbors) {
                                Destroy_Vec[remaining_neighbor] = true;
                            }
                        }
                        vertexID++;
                    }

                    /** Synchronization Point **/
                    pthread_barrier_wait(&barrier2);

                    /** if a neighbor is removed, remove it self **/
                    vertexID = vertex;
                    for(uint32_t i = 0; i < range; i++) {
                        if(Destroy_Vec[vertexID]) {
                            std::remove(Remaining_Vertices.begin(), Remaining_Vertices.end(), vertexID);
                        }
                        vertexID++;
                    }
                }));

                vertex += vertices_per_thread;
                thread_counter--; 
            }
        }

        for (auto &th : threadPool) {
            th.join();
        }
    }
}

std::string RandomPrioritySolver::name() const {
    return "RandomPrioritySolver (" + std::to_string(num_threads) + " threads)";
}