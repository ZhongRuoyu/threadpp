#include "thread_pool.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

void ThreadPool::Initialize() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    for (unsigned i = 0; i < workers_count_; ++i) {
        workers_.emplace_back(loop_);
    }
}

ThreadPool::ThreadPool()
    : ThreadPool::ThreadPool(std::thread::hardware_concurrency()) {}

ThreadPool::ThreadPool(const unsigned workers_count)
    : workers_count_(workers_count) {
    Initialize();
}

ThreadPool::~ThreadPool() {
    if (!terminated_) {
        ShutDown();
    }
}

unsigned ThreadPool::workers_count() { return workers_count_; }

void ThreadPool::Add(std::function<void()> f) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        jobs_.push(f);
    }
    condition_.notify_one();
}

void ThreadPool::Join() {
    terminate_ = true;
    condition_.notify_all();
    for (std::thread &worker : workers_) {
        worker.join();
    }
    workers_.clear();
    terminate_ = false;
    Initialize();
}

void ThreadPool::ShutDown() {
    terminate_ = true;
    condition_.notify_all();
    for (std::thread &worker : workers_) {
        worker.join();
    }
    workers_.clear();
    terminated_ = true;
}

void ThreadPool::ForceShutdown() {
    force_terminate_ = true;
    condition_.notify_all();
    for (std::thread &worker : workers_) {
        worker.join();
    }
    workers_.clear();
    while (!jobs_.empty()) {
        jobs_.pop();
    }
    terminated_ = true;
}
