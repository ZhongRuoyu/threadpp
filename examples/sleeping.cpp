#include <functional>
#include <iostream>
#include <sstream>
#include <thread>

#include "thread_pool.h"

auto start = std::chrono::steady_clock::now();

std::function<void()> ProduceFunction(int id, int millis) {
    return [=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(millis));
        auto now = std::chrono::steady_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - start)
                .count();
        std::stringstream stream;
        stream << "at " << duration << "ms: job " << id << " slept for "
               << millis << "ms" << std::endl;
        std::cout << stream.str();
    };
}

int main() {
    ThreadPool pool(3);
    std::cout << pool.workers_count() << " worker(s) in the pool" << std::endl;
    pool.Add(ProduceFunction(1, 100));
    pool.Add(ProduceFunction(2, 200));
    pool.Add(ProduceFunction(3, 300));
    pool.Add(ProduceFunction(4, 400));
    pool.Add(ProduceFunction(5, 500));
    pool.Add(ProduceFunction(6, 600));
    pool.Add(ProduceFunction(7, 700));
    pool.Add(ProduceFunction(8, 800));
    pool.Add(ProduceFunction(9, 900));
    pool.Add(ProduceFunction(10, 1000));
    pool.Join();
    auto now = std::chrono::steady_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - start)
            .count();
    std::cout << "at " << duration << "ms: pool joined" << std::endl;
}
