#ifndef GRAPH_COLORING_PCVECTOR_H
#define GRAPH_COLORING_PCVECTOR_H

#include <vector>
#include <optional>
#include <mutex>
#include <semaphore.h>

// A vector that implements the producer-consumer pattern.
template<typename T>
class PCVector {
    std::vector<T> data;
    bool stopped;

    std::mutex mutex;
    sem_t full;

public:
    PCVector() {
        sem_init(&full, 0, 0);
    }

    // Produce item val
    void push(const T& val) {
        mutex.lock();
        data.emplace_back(val);
        mutex.unlock();
        sem_post(&full);
    };

    // Consume an item; if the queue is empty, return std::nullopt
    std::optional<T> try_pop() {
        int status = sem_trywait(&full);
        if (status == -1)
            return std::nullopt;
        mutex.lock();
        T ret = data.back();
        data.pop_back();
        mutex.unlock();
        return ret;
    };

    // Signal that there are no more items to be produced
    void stop() {
        stopped = true;
    };

    // Check if there is no more work to be done
    bool done() const {
        return stopped && data.empty();
    };
};

#endif //GRAPH_COLORING_PCVECTOR_H
