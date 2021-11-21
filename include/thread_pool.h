#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
    const unsigned workers_count_;
    std::atomic<bool> terminate_{false};
    std::atomic<bool> force_terminate_{false};
    std::atomic<bool> terminated_{false};
    std::condition_variable condition_;

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> jobs_{};
    std::mutex queue_mutex_;

    /**
     * @brief The loop run by the workers. Jobs are automatically assigned on a
     * first-come, first-served basis.
     */
    const std::function<void()> loop_ = [&]() {
        while (true) {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                condition_.wait(lock, [&]() {
                    return !jobs_.empty() || terminate_ || force_terminate_;
                });
                if (terminate_ && jobs_.empty() || force_terminate_) {
                    break;
                }
                job = jobs_.front();
                jobs_.pop();
            }
            job();
        }
    };

    /**
     * @brief Initialize the thread pool.
     */
    void Initialize();

   public:
    ThreadPool(ThreadPool &pool) = delete;

    /**
     * @brief Construct a new thread pool with a default number of workers.
     */
    explicit ThreadPool();

    /**
     * @brief Construct a new thread pool with a specified number of workers.
     * @param workers_count the number of workers in the thread pool
     */
    explicit ThreadPool(const unsigned workers_count);

    /**
     * @brief Destroy the thread pool.
     */
    ~ThreadPool();

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
    void Add(std::function<void()> f);

    /**
     * @brief Wait for all the jobs in the queue to finish. The pool can be
     * reused until shutdown.
     */
    void Join();

    /**
     * @brief Shut down the thread pool gracefully. Unfinished jobs are finished
     * before the shutdown. After the shutdown, the pool cannot be used again.
     */
    void ShutDown();

    /**
     * @brief Shut down the thread pool. Jobs in progress are allowed to run
     * until the end, while unstarted jobs are discarded. After the shutdown,
     * the pool cannot be used again.
     */
    void ForceShutdown();
};

#endif /* THREAD_POOL_H_ */
