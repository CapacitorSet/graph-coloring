#ifndef GRAPH_COLORING_MEMORYMONITOR_H
#define GRAPH_COLORING_MEMORYMONITOR_H

#include <cstdint>
#include <thread>

class MemoryMonitor {
    int64_t peak_usage, baseline;
    bool stopped;

    std::thread thread;

    static void thread_function(MemoryMonitor &);

  public:
    MemoryMonitor();
    void stop();

    // Returns the total memory usage for the process
    int64_t total() const;
    // Returns the increase in memory usage since the monitor was started
    int64_t delta() const;

    static int64_t current_mem_usage();
};

#endif //GRAPH_COLORING_MEMORYMONITOR_H
