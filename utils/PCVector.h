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
    sem_t full_or_done;

public:
    PCVector() : stopped(false) {
        sem_init(&full_or_done, 0, 0);
    }
    PCVector(std::vector<T> &&_data) : data(std::move(_data)), stopped(false) {
        sem_init(&full_or_done, 0, data.size());
    }
    ~PCVector() {
        sem_destroy(&full_or_done);
    }

    // Produce item val
    void push(const T& val) {
        if (stopped)
            throw std::runtime_error("Writing to stopped queue");
        mutex.lock();
        data.emplace_back(val);
        mutex.unlock();
        sem_post(&full_or_done);
    };

    // Consume an item; if the queue is empty, wait until something is pushed
    std::optional<T> pop() {
        sem_wait(&full_or_done);
        if (stopped) {
            // Reincrement the semaphore to wake up other threads
            sem_post(&full_or_done);
            return std::nullopt;
        }
        mutex.lock();
        T ret = data.back();
        data.pop_back();
        mutex.unlock();
        return ret;
    };

    // Consume an item; if the queue is empty, return std::nullopt
    std::optional<T> try_pop() {
        if (empty())
            return std::nullopt;
        sem_wait(&full_or_done);
        if (stopped) {
            // Reincrement the semaphore to wake up other threads
            sem_post(&full_or_done);
            return std::nullopt;
        }
        mutex.lock();
        T ret = data.back();
        data.pop_back();
        mutex.unlock();
        return ret;
    };

    // Signal that there are no more items to be produced
    // Do a "fictitious" post on full_or_done so it can wake up threads waiting for an element
    void stop() {
        stopped = true;
        sem_post(&full_or_done);
    };

    // Check if there is no more work to be done. Not to be confused with stop()
    [[nodiscard]] bool done() const {
        return stopped && data.empty();
    };

    bool empty() const {
        return data.empty();
    };
};

#endif //GRAPH_COLORING_PCVECTOR_H
