#include <functional>
#include <iostream>
#include <sstream>
#include <thread>

#include "thread_pool.h"

std::function<void()> ProduceFunction(int id) {
    return [=]() {
        std::stringstream stream;
        stream << "job " << id << " handled by worker "
               << std::this_thread::get_id() << std::endl;
        std::cout << stream.str();
    };
}

int main() {
    ThreadPool pool;
    unsigned n = pool.workers_count();
    std::cout << n << " worker(s) in the pool" << std::endl;
    for (unsigned i = 0; i < n; ++i) {
        pool.Add(ProduceFunction(i));
    }
    pool.Join();
    std::cout << "pool joined" << std::endl;
    for (unsigned i = 0; i < n; ++i) {
        pool.Add(ProduceFunction(i));
    }
    pool.ShutDown();
    std::cout << "pool shut down" << std::endl;
}
