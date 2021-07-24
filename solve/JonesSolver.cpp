#include "JonesSolver.h"
#include <algorithm>
#include <shared_mutex>
#include <set>
#include <csignal>

JonesSolver::JonesSolver(int num_threads)
        : num_threads(num_threads), gen(RANDOM_SEED), rho_barrier(),
          thread_queue(num_threads), queue_mutex(num_threads), thread_cv(num_threads) {
    pthread_barrier_init(&rho_barrier, nullptr, num_threads);
}

JonesSolver::~JonesSolver() {
    pthread_barrier_destroy(&rho_barrier);
}

std::string JonesSolver::name() const {
    return "JonesSolver (" + std::to_string(num_threads) + " threads)";
}

void JonesSolver::solve(Graph &graph) {
    rho.resize(graph.vertices.size());
    // In Jones' paper, "choose rho(v)" = generate a different random number for each vertex
    std::iota(rho.begin() + 1, rho.end(), 1);
    std::shuffle(rho.begin(), rho.end(), gen);

    for (uint32_t i = 0; i < num_threads; i++) {
        threads.emplace_back(thread_function, i, &graph, std::ref(*this));
    }

    for (auto &t : threads)
        t.join();
}

// Vertices are split sequentially among threads.
// Eg. if there are four threads the first will handle vertices 0, 4, 8..., the second will handle 1, 5, 9...
void JonesSolver::thread_function(uint32_t thread_idx, Graph *graph, JonesSolver &solver) {
    std::unordered_map<uint32_t, ColoringProcess> processes(graph->vertices.size());
    std::vector<uint32_t> &queue = solver.thread_queue[thread_idx];
    std::mutex &queue_mutex = solver.queue_mutex[thread_idx];
    std::condition_variable &cv = solver.thread_cv[thread_idx];

    // For each vertex handled by this thread
    for (uint32_t v = thread_idx; v < graph->vertices.size(); v += solver.num_threads)
        // Create the "process" - just its data structures actually, control flow remains in this thread
        processes.emplace(v, std::move(ColoringProcess(v, solver, *graph)));
    pthread_barrier_wait(&solver.rho_barrier);

    for (auto &pair : processes) {
        auto vertex = pair.first;
        ColoringProcess &process = pair.second;
        if (process.waitlist.empty()) {
            graph->color_with_smallest(vertex);
            solver.notify_vertex_changed(vertex, *graph);
        }
    }

    while (!processes.empty() && !std::all_of(processes.cbegin(), processes.cend(),
                                              [](const std::pair<uint32_t, ColoringProcess> &p) { return p.second.waitlist.empty(); })) {
        std::unique_lock<std::mutex> lk(queue_mutex);
        cv.wait(lk, [&]() { return !queue.empty(); });
        // We use a while loop (rather than looping once) so we can add elements to the queue in notify_vertex_changed
        // and process them immediately, without going through the condition variable
        while (!queue.empty()) {
            uint32_t colored = queue.back();
            queue.pop_back();
            // We no longer need access to the queue; we'll re-lock it at the end of the while body
            lk.unlock();

            // "Send" the color to this vertex's neighbors.
            for (uint32_t neighbor : graph->neighbors_of(colored)) {
                if (solver.thread_for(neighbor) != thread_idx)
                    continue;
                ColoringProcess &process = processes.at(neighbor);
                if (process.waitlist.empty())
                    continue;
                bool done = process.receive(colored);
                if (done) {
                    // All the neighboring nodes were colored, so we can color the main node now
                    graph->color_with_smallest(neighbor);
                    solver.notify_vertex_changed(neighbor, *graph);
                }
            }
            lk.lock();
        }
    }
}

// Add the changed vertex into the queue of each neighbor's thread
void JonesSolver::notify_vertex_changed(uint32_t changed, const Graph &graph) {
    // Avoid notifying the same thread several times if it owns several neighbors
    std::bitset<4096> was_notified;
    for (const auto &neighbor : graph.neighbors_of(changed)) {
        uint32_t thread_idx = thread_for(neighbor);
        if (was_notified.test(thread_idx))
            continue;
        was_notified.set(thread_idx);
        // We don't have to worry about recursive locks, since thread_function releases the lock as soon as it pops an item from the queue
        std::unique_lock<std::mutex> lk(queue_mutex[thread_idx]);
        thread_queue[thread_idx].emplace_back(changed);
        thread_cv[thread_idx].notify_one();
    }
}

uint32_t JonesSolver::thread_for(uint32_t vertex) const {
    return vertex % num_threads;
}

ColoringProcess::ColoringProcess(uint32_t v, const JonesSolver &solver, const Graph &graph) {
    for (uint32_t w : graph.neighbors_of(v))
        if (solver.rho[w] > solver.rho[v])
            waitlist.emplace_back(w);
    // Note that we do not use a send_queue: we just check when all neighbors have been colored
}

bool ColoringProcess::receive(uint32_t v) {
    // We can use lower_bound since the waitlist is a sorted vector of neighbors
    auto pos = std::lower_bound(waitlist.cbegin(), waitlist.cend(), v);
    // Todo: remove
    if (pos == waitlist.cend())
        raise(SIGTRAP);
    waitlist.erase(pos);
    return waitlist.empty();
}
