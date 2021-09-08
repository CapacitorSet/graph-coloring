#include "JonesSolver.h"

JonesSolver::JonesSolver(int num_threads) : num_threads(num_threads), gen(RANDOM_SEED) {}

std::string JonesSolver::name() const {
    return "JonesSolver (" + std::to_string(num_threads) + " threads)";
}

void JonesSolver::solve(Graph &graph) {
    std::vector<uint32_t> rho(graph.vertices.size());
    // In Jones' paper, "choose rho(v)" = generate a different random number for each vertex
    std::iota(rho.begin() + 1, rho.end(), 1);
    std::shuffle(rho.begin(), rho.end(), gen);

    PCVector<uint32_t> free_vertices;
    std::atomic<uint32_t> num_vertices_uncolored = graph.vertices.size();

    // Associates each vertex with the number of neighbors it is "waiting on" (uncolored with higher rho)
    std::vector<std::atomic<int>> waitlist(graph.vertices.size());
    for (uint32_t vertex = 0; vertex < graph.vertices.size(); ++vertex) {
        const edges_t &neighbors = graph.neighbors_of(vertex);
        auto rho_current = rho[vertex];
        int num_wait = 0;
        for (const auto neighbor : graph.neighbors_of(vertex))
            if (rho[neighbor] > rho_current)
                num_wait++;
        waitlist[vertex] = num_wait;
        if (num_wait == 0)
            free_vertices.push(vertex);
    }

    free_vertices.onReceive(num_threads, [&free_vertices, &num_vertices_uncolored, &graph, &waitlist, &rho](uint32_t vertex) {
        // Check if there are no vertices left to color
        if (--num_vertices_uncolored == 0)
            // If so, tell all threads not to wait for more free vertices
            free_vertices.stop();

        // Color the current node...
        graph.color_with_smallest(vertex);
        // And update any neighbor that may be "waiting" on it
        for (uint32_t neighbor: graph.neighbors_of(vertex)) {
            // If it is no longer waiting on anything, push it to the free vertices queue
            if (rho[vertex] > rho[neighbor] && --waitlist[neighbor] == 0)
                free_vertices.push(neighbor);
        }
        /*
        // Deadlock detection
        if (num_threads == 1 && free_vertices.empty() && num_vertices_uncolored != 0)
            raise(SIGTRAP);
        */
    });

    free_vertices.join();
}
