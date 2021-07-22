#include <chrono>
#include <iostream>
#include <fstream>
#if __linux__
#include <unistd.h>
#endif
#include "Benchmark.h"
#include "../solve/SequentialSolver.h"
#include "../solve/LubySolver.h"
#include "../solve/JonesSolver.h"

Benchmark::Benchmark(Graph &g) : solvers({
    new SequentialSolver(),
    new LubySolver(),
    new JonesSolver(1),
    new JonesSolver(2),
    new JonesSolver(4),
}), graph(g) {}

void Benchmark::run() {
    for (Solver *s : solvers) {
        std::cout << s->name() << ":" << std::endl;
        struct result res = run_single(s);
        printf("%.2f ms, %.2f MB (%s, %d colors)\n\n",
               res.milliseconds,
               double(res.peak_mem_usage) / 1024 / 1024,
               res.success ? "success" : "fail",
               res.num_colors);
    }
}

struct result Benchmark::run_single(Solver *solver) {
    // Used to receive memory info from the thread
    std::promise<uint64_t> mem_usage_p;
    // Used to tell the thread when to stop measuring
    std::promise<void> stop;
    std::future<void> stop_f = stop.get_future();
    uint64_t baseline_mem_usage = 0;current_mem_usage();

    std::thread mem_monitor_thread(mem_monitor_thread_function, std::ref(mem_usage_p), std::ref(stop_f));
    auto t1 = std::chrono::high_resolution_clock::now();
    solver->solve(graph);
    auto t2 = std::chrono::high_resolution_clock::now();
    stop.set_value();
    mem_monitor_thread.join();

    bool success = graph.is_well_colored();
    uint32_t num_colors = graph.count_colors();
    double milliseconds = std::chrono::duration<double, std::milli>(t2 - t1).count();
    // Subtract the memory usage from other data structures in the program
    uint64_t mem_usage = mem_usage_p.get_future().get() - baseline_mem_usage;
    graph.clear();
    return {success, num_colors, milliseconds, mem_usage};
}

void Benchmark::mem_monitor_thread_function(std::promise<uint64_t> &result, std::future<void> &stop) {
    uint64_t peak_mem_usage = 0;
    std::future_status status;
    do {
        uint64_t mem_usage = current_mem_usage();
        if (mem_usage > peak_mem_usage)
            peak_mem_usage = mem_usage;
        // Wait for the solver to stop, for at most 0.1 ms; then measure the memory usage again
        status = stop.wait_for(std::chrono::microseconds(100));
    } while (status != std::future_status::ready);
    result.set_value(peak_mem_usage);
}

uint64_t Benchmark::current_mem_usage() {
#if __linux__
    // https://stackoverflow.com/a/42925322
    std::ifstream statm("/proc/self/statm");
    uint64_t token;
    // Skip tokens 1-5
    for (int i = 0; i < 5; i++)
        statm >> token;
    // Read token 6, "data + stack"
    statm >> token;
    return token*getpagesize();
#else
    return 0;
#endif
}
