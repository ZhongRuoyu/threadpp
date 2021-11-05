#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class thread_pool {
    const unsigned int num_workers;
    std::atomic<bool> terminate{false};
    std::atomic<bool> force_terminate{false};
    std::atomic<bool> terminated{false};
    std::condition_variable condition;

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> jobs{};
    std::mutex queue_mutex;

    /**
     * @brief The loop run by the workers. Jobs are automatically assigned on a
     * first-come, first-served basis.
     */
    const std::function<void()> loop = [&]() {
        while (true) {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                condition.wait(lock, [&]() {
                    return !jobs.empty() || terminate || force_terminate;
                });
                if (terminate && jobs.empty() || force_terminate) {
                    break;
                }
                job = jobs.front();
                jobs.pop();
            }
            job();
        }
    };

    /**
     * @brief Initialize the thread pool.
     */
    void initialize();

   public:
    thread_pool(thread_pool &pool) = delete;

    /**
     * @brief Construct a new thread pool with a default number of workers.
     */
    explicit thread_pool();

    /**
     * @brief Construct a new thread pool with a specified number of workers.
     * @param num_workers the number of workers in the thread pool
     */
    explicit thread_pool(const unsigned int num_workers);

    /**
     * @brief Destroy the thread pool.
     */
    ~thread_pool();

    /**
     * @brief Get the number of workers in the pool.
     * @return the number of workers in the pool
     */
    unsigned workers_count();

    /**
     * @brief Add a job to the pool's job queue. Jobs are handled on a
     * first-come, first-served basis. The job must be a void function with no
     * arguments.
     * @param f the job to be added
     */
    void add(std::function<void()> f);

    /**
     * @brief Wait for all the jobs in the queue to finish. The pool can be
     * reused until shutdown.
     */
    void join();

    /**
     * @brief Shut down the thread pool gracefully. Unfinished jobs are finished
     * before the shutdown. After the shutdown, the pool cannot be used again.
     */
    void shutdown();

    /**
     * @brief Shut down the thread pool. Jobs in progress are allowed to run
     * until the end, while unstarted jobs are discarded. After the shutdown,
     * the pool cannot be used again.
     */
    void force_shutdown();
};

#endif /* THREAD_POOL_H_ */
