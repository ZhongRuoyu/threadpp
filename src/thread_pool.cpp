#include "thread_pool.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

/**
 * @brief Initialize the thread pool.
 */
void thread_pool::initialize() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    for (unsigned int i = 0; i < num_workers; ++i) {
        workers.emplace_back(loop);
    }
}

/**
 * @brief Construct a new thread pool with a default number of workers.
 */
thread_pool::thread_pool()
    : thread_pool::thread_pool(std::thread::hardware_concurrency()) {}

/**
 * @brief Construct a new thread pool with a specified number of workers.
 * @param num_workers the number of workers in the thread pool
 */
thread_pool::thread_pool(const unsigned int num_workers)
    : num_workers(num_workers) {
    initialize();
}

/**
 * @brief Destroy the thread pool.
 */
thread_pool::~thread_pool() {
    if (!terminated) {
        shutdown();
    }
}

/**
 * @brief Get the number of workers in the pool.
 * @return the number of workers in the pool
 */
unsigned thread_pool::workers_count() { return num_workers; }

/**
 * @brief Add a job to the pool's job queue. Jobs are handled on a
 * first-come, first-served basis. The job must be a void function with no
 * arguments.
 * @param f the job to be added
 */
void thread_pool::add(std::function<void()> f) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        jobs.push(f);
    }
    condition.notify_one();
}

/**
 * @brief Wait for all the jobs in the queue to finish. The pool can be
 * reused until shutdown.
 */
void thread_pool::join() {
    terminate = true;
    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
    workers.clear();
    terminate = false;
    initialize();
}

/**
 * @brief Shut down the thread pool gracefully. Unfinished jobs are finished
 * before the shutdown. After the shutdown, the pool cannot be used again.
 */
void thread_pool::shutdown() {
    terminate = true;
    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
    workers.clear();
    terminated = true;
}

/**
 * @brief Shut down the thread pool. Jobs in progress are allowed to run
 * until the end, while unstarted jobs are discarded. After the shutdown,
 * the pool cannot be used again.
 */
void thread_pool::force_shutdown() {
    force_terminate = true;
    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
    workers.clear();
    while (!jobs.empty()) {
        jobs.pop();
    }
    terminated = true;
}
