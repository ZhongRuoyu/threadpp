#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

namespace threadpp {

class ThreadPool {
   public:
    ThreadPool()
        : ThreadPool::ThreadPool(std::thread::hardware_concurrency()) {}
    ThreadPool(const ThreadPool &pool) = delete;
    explicit ThreadPool(unsigned int workers_count);

    ~ThreadPool();

    template <class F, class... Args>
    std::future<typename std::result_of<F(Args...)>::type> Add(F &&f,
                                                               Args &&...args);

    void Join();

    unsigned int workers_count() const { return this->workers_count_; }

   private:
    void Loop();

    const unsigned int workers_count_;
    std::atomic<bool> join_{false};
    std::atomic<bool> joined_{false};
    std::condition_variable condition_;
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> jobs_;
    std::mutex queue_mutex_;
};

template <class F, class... Args>
std::future<typename std::result_of<F(Args...)>::type> ThreadPool::Add(
    F &&f, Args &&...args) {
    if (this->join_) {
        throw std::runtime_error("Adding a job to a joined ThreadPool.");
    }

    using ResultType = typename std::result_of<F(Args...)>::type;

    auto job = std::make_shared<std::packaged_task<ResultType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<ResultType> result = job->get_future();

    {
        std::unique_lock<std::mutex> lock(this->queue_mutex_);
        this->jobs_.push([job]() { (*job)(); });
    }
    this->condition_.notify_one();

    return result;
}

}  // namespace threadpp

#endif  // THREAD_POOL_H_
