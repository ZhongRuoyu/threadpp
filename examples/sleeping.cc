#include <thread_pool.h>

#include <chrono>
#include <functional>
#include <iostream>
#include <sstream>
#include <thread>

auto start = std::chrono::steady_clock::now();

int main() {
    threadpp::ThreadPool pool(3);
    std::cout << pool.workers_count() << " worker(s) in the pool" << std::endl;
    for (unsigned i = 0; i != 10; ++i) {
        pool.Add([=]() {
            unsigned millis = i * 100;
            std::this_thread::sleep_for(std::chrono::milliseconds(millis));
            auto now = std::chrono::steady_clock::now();
            auto duration =
                std::chrono::duration_cast<std::chrono::milliseconds>(now -
                                                                      start)
                    .count();
            std::stringstream stream;
            stream << "at " << duration << "ms: job " << i << " slept for "
                   << millis << "ms" << std::endl;
            std::cout << stream.str();
        });
    }
    pool.Join();
    auto now = std::chrono::steady_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - start)
            .count();
    std::cout << "at " << duration << "ms: pool joined" << std::endl;
}
