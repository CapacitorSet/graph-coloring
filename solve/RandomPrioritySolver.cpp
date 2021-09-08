#include "RandomPrioritySolver.h"

RandomPrioritySolver::RandomPrioritySolver(int num_threads) : num_threads(num_threads) {}

void RandomPrioritySolver::solve(Graph &original_graph) {
    DeletableGraph uncolored_graph(original_graph);
    color_t color = 0;
    while (!uncolored_graph.empty()) {
        compute_MIS(uncolored_graph);
        for (uint32_t vertex : MIS) {
            original_graph.colors[vertex] = color;
            uncolored_graph.delete_vertex(vertex);
        }
        color++;
    }
}

// Best explained here:
// https://en.wikipedia.org/wiki/Maximal_independent_set#Random-priority_parallel_algorithm

void RandomPrioritySolver::compute_MIS(const DeletableGraph &del_graph) {
    // Reset solver state
    MIS.clear();
    Remaining_Vertices.clear();

    const Graph &graph = del_graph.graph;
    uint32_t num_verticies_graph = graph.vertices.size();

    for (uint32_t i = 0; i < num_verticies_graph; i++) {
        // It suffices to check for is_deleted here, since we don't delete vertices inside the function
        if (!del_graph.is_deleted(i)) {
            Remaining_Vertices.emplace_back(i);
        }
    }

    uint32_t num_vertices_Remaining = Remaining_Vertices.size();

    Random_Priority_Vec.reserve(num_verticies_graph);

    Destroy_Vec.assign(num_verticies_graph, false);

    while (!Remaining_Vertices.empty()) {
        std::vector<std::thread> threadPool;

        pthread_barrier_init(&barrier1, NULL, num_vertices_Remaining);
        pthread_barrier_init(&barrier2, NULL, num_vertices_Remaining);

        for (uint32_t thID = 0; thID < num_vertices_Remaining; thID++) {
            threadPool.emplace_back(std::thread([&thID, &graph, this] () {

                uint32_t vertexID = Remaining_Vertices[thID];

                // Select random number
                uint32_t RandNum = rand() % 100;

                // Send it to the neighbours "Announce it"
                Random_Priority_Vec[vertexID] = RandNum;

                edges_t neighbors = graph.neighbors_of(vertexID);
                edges_t remaining_neighbors;
                bool IamTheSmallest = true;

                pthread_barrier_wait(&barrier1);

                for (uint32_t neighbor : neighbors) {
                    if (std::find(Remaining_Vertices.begin(), Remaining_Vertices.end(), neighbor) != Remaining_Vertices.end()) {
                        remaining_neighbors.emplace_back(neighbor);
                        if (Random_Priority_Vec[neighbor] < RandNum) {
                            IamTheSmallest = false;
                            break;
                        }
                    }
                }

                if (IamTheSmallest) {

                    // Insert itself into MIS
                    wrt_mutex.lock();
                    MIS.emplace_back(vertexID);
                    wrt_mutex.unlock();

                    // Remove itself from Remaining_Vertices
                    std::remove(Remaining_Vertices.begin(), Remaining_Vertices.end(), vertexID);

                    // Tells the neighbors
                    for (uint32_t neighbor : remaining_neighbors) {
                        wrt_mutex.lock();
                        Destroy_Vec[neighbor] = true;
                        wrt_mutex.unlock();
                    }
                }

                pthread_barrier_wait(&barrier2);

                if (Destroy_Vec[vertexID] == true) {
                    // Remove itself from Remaining_Vertices
                    std::remove(Remaining_Vertices.begin(), Remaining_Vertices.end(), vertexID);
                }
            }));
        }

        for (auto &th : threadPool) {
            th.join();
        }
    }
}

std::string RandomPrioritySolver::name() const {

    return "RandomPrioritySolver (" + std::to_string(num_threads) + " threads)";

}



