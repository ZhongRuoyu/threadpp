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
#include <type_traits>
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
    std::future<
        typename std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
    Add(F &&f, Args &&...args);

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

namespace detail {

template <class T>

std::decay_t<T> DecayCopy(T &&v) {
    return std::forward<T>(v);
}

}  // namespace detail

template <class F, class... Args>
std::future<
    typename std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
ThreadPool::Add(F &&f, Args &&...args) {
    if (this->join_) {
        throw std::runtime_error("Adding a job to a joined ThreadPool.");
    }

    using ResultType =
        typename std::invoke_result_t<std::remove_reference_t<F>,
                                      std::remove_reference_t<Args>...>;

    auto job = std::make_shared<std::packaged_task<ResultType()>>(
        std::bind(detail::DecayCopy(std::forward<F>(f)),
                  detail::DecayCopy(std::forward<Args>(args))...));
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
