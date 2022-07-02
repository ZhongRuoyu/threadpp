#include "thread_pool.h"

#include <atomic>
#include <condition_variable>
#include <exception>
#include <functional>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

namespace threadpp {

ThreadPool::ThreadPool(const unsigned int workers_count)
    : workers_count_(workers_count), join_signal_(false), joinable_(true) {
    std::unique_lock<std::mutex> lock(this->queue_mutex_);
    for (unsigned int i = 0; i != workers_count; ++i) {
        this->workers_.emplace_back(std::bind(&ThreadPool::Loop, this));
    }
}

void ThreadPool::Join() {
    if (!this->joinable_.exchange(false)) {
        std::terminate();
    }
    this->join_signal_ = true;
    this->condition_.notify_all();
    for (auto &worker : this->workers_) {
        worker.join();
    }
    {
        std::unique_lock<std::mutex> lock(this->queue_mutex_);
        while (!this->jobs_.empty()) {
            auto job = this->jobs_.front();
        }
    }
    this->workers_.clear();
    this->join_signal_ = false;
}

void ThreadPool::Loop() {
    for (;;) {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(this->queue_mutex_);
            this->condition_.wait(lock, [this] {
                return !this->jobs_.empty() || this->join_signal_;
            });
            if (this->join_signal_) {
                break;
            }
            job = this->jobs_.front();
            this->jobs_.pop();
        }
        job();
    }
}

}  // namespace threadpp
