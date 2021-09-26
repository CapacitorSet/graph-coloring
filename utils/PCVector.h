#ifndef GRAPH_COLORING_PCVECTOR_H
#define GRAPH_COLORING_PCVECTOR_H

#include <functional>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>
#if __linux__
#include <pthread.h>
#endif

// A vector that implements the producer-consumer pattern.
template <typename T>
class PCVector {
  public:
    using callback_t = void(T);

  private:
    std::vector<T> data;
    bool stopped;

    std::mutex mutex;
    sem_t full_or_done;

    // Threads used by onReceive
    std::vector<std::thread> threads;

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
    void push(const T &val) {
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
            // Note that even if the queue is stopped we must check if there are any items left!
        }
        std::unique_lock lock(mutex);
        if (done())
            return std::nullopt;
        T ret = data.back();
        data.pop_back();
        return ret;
    };

    void onReceive(int num_threads, callback_t *callback) {
        for (int i = 0; i < num_threads; i++)
            threads.emplace_back([](PCVector<T> &queue, callback_t *callback, int i) {
#if __linux__
                // For debugging
                std::string thread_name = "onReceive#" + std::to_string(i);
                pthread_setname_np(pthread_self(), thread_name.c_str());
#endif
                while (std::optional<T> item = queue.pop()) {
                    callback(*item);
                }
            },
                                 std::ref(*this), callback, i);
    }

    void onReceive(int num_threads, std::function<callback_t> callback) {
        for (int i = 0; i < num_threads; i++)
            threads.emplace_back([=](PCVector<T> &queue, int i) {
#if __linux__
                // For debugging
                std::string thread_name = "onReceive#" + std::to_string(i);
                pthread_setname_np(pthread_self(), thread_name.c_str());
#endif
                while (std::optional<T> item = queue.pop()) {
                    callback(*item);
                }
            },
                                 std::ref(*this), i);
    }

    // Signal that there are no more items to be produced
    // Do a "fictitious" post on full_or_done so it can wake up threads waiting for an element
    void stop() {
        stopped = true;
        sem_post(&full_or_done);
    };

    // Join any onReceive thread
    void join() {
        for (auto &t : threads)
            t.join();
    }

    // Check if there is no more work to be done. Not to be confused with stop()
    [[nodiscard]] bool done() const {
        return stopped && data.empty();
    };

    bool empty() const {
        return data.empty();
    };
};

#endif //GRAPH_COLORING_PCVECTOR_H
