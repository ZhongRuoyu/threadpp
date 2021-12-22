#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
    const unsigned workers_count_;
    std::atomic<bool> join_{false};
    std::atomic<bool> joined_{false};
    std::condition_variable condition_;

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> jobs_{};
    std::mutex queue_mutex_;

    /**
     * @brief The loop run by the workers. Jobs are automatically assigned on a
     * first-come, first-served basis.
     */
    const std::function<void()> loop_ = [&] {
        while (true) {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                condition_.wait(lock, [&] { return !jobs_.empty() || join_; });
                if (join_ && jobs_.empty()) {
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
    explicit ThreadPool()
        : ThreadPool::ThreadPool(std::thread::hardware_concurrency()) {}

    /**
     * @brief Construct a new thread pool with a specified number of workers.
     * @param workers_count the number of workers in the thread pool
     */
    explicit ThreadPool(const unsigned workers_count)
        : workers_count_(workers_count) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        for (unsigned i = 0; i < workers_count_; ++i) {
            workers_.emplace_back(loop_);
        }
    }

    /**
     * @brief Destroy the thread pool.
     */
    ~ThreadPool() {
        if (!joined_) {
            Join();
        }
    }
    /**
     * @brief Get the number of workers in the pool.
     * @return the number of workers in the pool
     */
    unsigned workers_count() {
        return workers_count_;
    }

    /**
     * @brief Add a job to the pool's job queue. Jobs are handled on a
     * first-come, first-served basis.
     * @tparam F the type of the job function
     * @tparam Args the types of arguments provided to the job function
     * @param f the job to be added
     * @param args the arguments provided to the job function
     * @return the result of the job, wrapped in a std::future
     */
    template <class F, class... Args>
    inline auto Add(F &&f, Args &&...args)
        -> std::future<typename std::result_of<F(Args...)>::type> {
        if (join_) {
            throw std::runtime_error("Adding a job to a joined ThreadPool.");
        }

        using result_type = typename std::result_of<F(Args...)>::type;

        auto job = std::make_shared<std::packaged_task<result_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<result_type> result = job->get_future();

        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            jobs_.push([=]() { (*job)(); });
        }
        condition_.notify_one();

        return result;
    }

    /**
     * @brief Shut down the thread pool gracefully. Unfinished jobs are finished
     * before the shutdown. After the shutdown, the pool cannot be used again.
     */
    void Join() {
        if (joined_) {
            throw std::runtime_error("Joining a joined ThreadPool.");
        }
        join_ = true;
        condition_.notify_all();
        for (std::thread &worker : workers_) {
            worker.join();
        }
        workers_.clear();
        joined_ = true;
    }
};

#endif /* THREAD_POOL_H_ */
