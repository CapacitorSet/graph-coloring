#include <chrono>
#include <iostream>
#include "Benchmark.h"

Benchmark::Benchmark(Graph &g) : sequential(), luby(), graph(g) {}

void Benchmark::run() {
    std::cout << "Running: " << sequential.name << std::endl;
    struct result seq_res = run_single(&sequential);
    printf("%s: %.2f ms (%s, %d colors)\n",
           sequential.name.c_str(), seq_res.milliseconds,
           seq_res.success ? "success" : "fail", seq_res.num_colors);

    std::cout << "Running: " << luby.name << std::endl;
    struct result luby_res = run_single(&luby);
    printf("%s: %.2f ms (%s, %d colors)\n",
           sequential.name.c_str(), luby_res.milliseconds,
           luby_res.success ? "success" : "fail", luby_res.num_colors);
}

struct result Benchmark::run_single(Solver *solver) {
    auto t1 = std::chrono::high_resolution_clock::now();
    solver->solve(graph);
    auto t2 = std::chrono::high_resolution_clock::now();
    bool success = graph.is_well_colored();
    uint32_t num_colors = graph.count_colors();
    graph.clear_colors();

    double milliseconds = std::chrono::duration<double, std::milli>(t2 - t1).count();
    return {success, num_colors, milliseconds};
}
