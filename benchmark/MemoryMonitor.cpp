#include <chrono>
#include <fstream>
#include <thread>

#if __linux__
#include <unistd.h>
#endif

#include "MemoryMonitor.h"

void MemoryMonitor::thread_function(MemoryMonitor &monitor) {
    do {
        int64_t mem_usage = MemoryMonitor::current_mem_usage();
        if (mem_usage > monitor.peak_usage)
            monitor.peak_usage = mem_usage;
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    } while (!monitor.stopped);
}

// https://stackoverflow.com/a/42925322
MemoryMonitor::MemoryMonitor() : stopped(false), baseline(current_mem_usage()), peak_usage(0), thread(thread_function, std::ref(*this)) {
#if __linux__
    // For debugging
    pthread_setname_np(thread.native_handle(), "MemoryMonitor");
#endif
}

int64_t MemoryMonitor::current_mem_usage() {
#if __linux__
    std::ifstream mem_fstream("/proc/self/statm");
    int64_t token;
    // Skip tokens 1-5
    for (int i = 0; i < 5; i++)
        mem_fstream >> token;
    // Read token 6, "data + stack"
    mem_fstream >> token;
    return token * getpagesize();
#else
    return 0;
#endif
}

void MemoryMonitor::stop() {
    stopped = true;
    thread.join();
}

int64_t MemoryMonitor::total() const { return peak_usage; }

int64_t MemoryMonitor::delta() const { return peak_usage - baseline; }
