#include <thread_pool.h>

#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

int main() {
    threadpp::ThreadPool pool;
    unsigned n = pool.workers_count();
    std::cout << n << " worker(s) in the pool" << std::endl;

    std::vector<std::future<unsigned>> results;
    for (unsigned i = 0; i != 2 * n; ++i) {
        results.push_back(pool.Add([i] {
            std::stringstream stream;
            stream << "job " << i << " handled by worker "
                   << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << stream.str();
            return i;
        }));
    }

    for (auto &result : results) {
        std::cout << result.get() << " ";
    }
    std::cout << std::endl;

    pool.Join();
}
