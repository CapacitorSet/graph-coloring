#include "LubySolver.h"

LubySolver::LubySolver(int num_threads) : num_threads(num_threads), partial_S(num_threads), kill_threads(false),
    V_splitter(nullptr), gen(RANDOM_SEED) {
    pthread_barrier_init(&thread_start_barrier, nullptr, num_threads + 1);
    pthread_barrier_init(&thread_end_barrier, nullptr, num_threads + 1);
}

LubySolver::~LubySolver() {
    pthread_barrier_destroy(&thread_start_barrier);
    pthread_barrier_destroy(&thread_end_barrier);
    delete V_splitter;
}

void LubySolver::solve(Graph &original_graph) {
    DeletableGraph uncolored_graph(original_graph);

    // We create the threads here, but they are only "activated" by the barrier in probabilistic_select.
    for (int i = 0; i < num_threads; i++)
        threads.emplace_back([&](int thread_idx, const Graph &graph) {
            while (true) {
                pthread_barrier_wait(&thread_start_barrier);
                // Used to prevent threads from looping forever
                if (kill_threads)
                    return;
                // Get the slice of V that this thread will work on
                auto partial_V = V_splitter->get(thread_idx);
                if (!partial_V.empty()) {
                    auto &S = partial_S[thread_idx];
                    // Select at least one item from V
                    while (S.empty()) {
                        // Include each vertex with probability 1/(2/d(v))
                        for (uint32_t vertex : partial_V) {
                            double probability = 1. / (2 * graph.degree_of(vertex));
                            std::bernoulli_distribution d(probability);
                            if (d(gen)) {
                                S.push_back(vertex);
                                S_bitmap[vertex] = true;
                            }
                        }
                    }
                }
                pthread_barrier_wait(&thread_end_barrier);
            }
        },
                             i, std::cref(original_graph));

    // Basic structure of MIS-based algorithms: create a MIS, then color it with a new color, and remove it from the graph
    color_t color = 0;
    while (!uncolored_graph.empty()) {
        compute_MIS(uncolored_graph);
        for (uint32_t vertex : MIS) {
            original_graph.colors[vertex] = color;
            uncolored_graph.delete_vertex(vertex);
        }
        color++;
    }

    kill_threads = true;
    pthread_barrier_wait(&thread_start_barrier);
    for (auto &t : threads)
        t.join();
}

// Best explained here:
// https://en.wikipedia.org/wiki/Maximal_independent_set#Random-selection_parallel_algorithm_[Luby's_Algorithm]
void LubySolver::compute_MIS(const DeletableGraph &del_graph) {
    // Reset solver state
    MIS.clear();
    V.clear();

    const Graph &graph = del_graph.graph;

    for (uint32_t i = 0; i < graph.num_vertices(); i++)
        // It suffices to check for is_deleted here, since we don't delete vertices inside the function
        if (!del_graph.is_deleted(i))
            V.emplace(i);

    while (!V.empty()) {
        // The subset of vertices selected
        // We use std::vector as the iterator for std::set is very slow
        probabilistic_select(graph);

        remove_edges(graph);

        for (uint32_t v : S) {
            if (!S_bitmap[v])
                continue;
            MIS.emplace_back(v);
            V.erase(v);
            for (uint32_t neighbor : graph.neighbors_of(v))
                V.erase(neighbor);
        }
    }
}

void LubySolver::probabilistic_select(const Graph &graph) {
    // Reset solver state
    S.clear();
    S_bitmap.clear();
    S_bitmap.resize(graph.num_vertices());
    for (auto &S_ : partial_S)
        S_.clear();
    // Mirror V into a temporary vector
    V_vec = std::move(std::vector<uint32_t>(V.cbegin(), V.cend()));
    if (V_splitter != nullptr)
        delete V_splitter;
    V_splitter = new VectorSplitter(V_vec, num_threads);

    // Start all threads...
    pthread_barrier_wait(&thread_start_barrier);
    // And wait for them to terminate
    pthread_barrier_wait(&thread_end_barrier);
    // Concatenate the partial S that each thread computed
    for (const auto &partial : partial_S)
        S.insert(S.end(), partial.cbegin(), partial.cend());
}

void LubySolver::remove_edges(const Graph &g) {
    // Optimization: do not check for edges if we have less than 2 nodes
    if (S.size() < 2)
        return;

    // Specification: "For each edge in E, check if 'from' and 'to' are in the graph S.".
    // We can loop on fewer items by checking "for each edge in S, check if 'from' and 'to' are in E."

    for (uint32_t from : S) {
        /* Equivalent to:
         *
         *   for (uint32_t to : g.neighbors_of(from))
         *
         * However, since neighbors are sorted we can start searching in the middle of the array, after the smaller vertices.
         */
        auto neighbors = g.neighbors_of(from);
        for (auto pos = std::lower_bound(neighbors.begin(), neighbors.end(), from); pos != neighbors.end(); ++pos) {
            uint32_t to = *pos;
            if (!S_bitmap[to])
                continue;
            if (g.degree_of(from) <= g.degree_of(to))
                S_bitmap[from] = false;
            else
                S_bitmap[to] = false;
        }
    }
}

std::string LubySolver::name() const {
    return "LubySolver (" + std::to_string(num_threads) + " threads)";
}
