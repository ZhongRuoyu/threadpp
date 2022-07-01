#include "thread_pool.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

namespace threadpp {

ThreadPool::ThreadPool(const unsigned int workers_count)
    : workers_count_(workers_count) {
    std::unique_lock<std::mutex> lock(this->queue_mutex_);
    for (unsigned int i = 0; i != workers_count; ++i) {
        this->workers_.emplace_back(std::bind(&ThreadPool::Loop, this));
    }
}

ThreadPool::~ThreadPool() {
    if (!this->joined_) {
        Join();
    }
}

void ThreadPool::Join() {
    if (this->joined_) {
        throw std::runtime_error("Joining a joined ThreadPool.");
    }
    this->join_ = true;
    this->condition_.notify_all();
    for (auto &worker : this->workers_) {
        worker.join();
    }
    this->workers_.clear();
    this->joined_ = true;
}

void ThreadPool::Loop() {
    for (;;) {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(this->queue_mutex_);
            this->condition_.wait(
                lock, [this] { return !this->jobs_.empty() || this->join_; });
            if (this->join_ && this->jobs_.empty()) {
                break;
            }
            job = this->jobs_.front();
            this->jobs_.pop();
        }
        job();
    }
}

}  // namespace threadpp
