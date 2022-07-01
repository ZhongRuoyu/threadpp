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

namespace threadpp {

class ThreadPool {
   public:
    ThreadPool(ThreadPool &pool) = delete;

    ThreadPool()
        : ThreadPool::ThreadPool(std::thread::hardware_concurrency()) {}

    explicit ThreadPool(const unsigned workers_count)
        : workers_count_(workers_count) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        for (unsigned i = 0; i != workers_count; ++i) {
            workers_.emplace_back(loop_);
        }
    }

    ~ThreadPool() {
        if (!joined_) {
            Join();
        }
    }

    unsigned workers_count() { return workers_count_; }

    template <class F, class... Args>
    auto Add(F &&f, Args &&...args)
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

    void Join() {
        if (joined_) {
            throw std::runtime_error("Joining a joined ThreadPool.");
        }
        join_ = true;
        condition_.notify_all();
        for (auto &worker : workers_) {
            worker.join();
        }
        workers_.clear();
        joined_ = true;
    }

   private:
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

    const unsigned workers_count_;
    std::atomic<bool> join_{false};
    std::atomic<bool> joined_{false};
    std::condition_variable condition_;
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> jobs_;
    std::mutex queue_mutex_;
};

}  // namespace threadpp

#endif  // THREAD_POOL_H_
