// Modified from https://en.cppreference.com/w/cpp/thread/thread/thread
// Licensed under CC-BY-SA.

#include <thread_pool.h>

#include <chrono>
#include <iostream>
#include <thread>
#include <utility>

void f1(int n) {
    for (int i = 0; i < 5; ++i) {
        std::cout << "Thread 1 executing\n";
        ++n;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void f2(int &n) {
    for (int i = 0; i < 5; ++i) {
        std::cout << "Thread 2 executing\n";
        ++n;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

class foo {
   public:
    void bar() {
        for (int i = 0; i < 5; ++i) {
            std::cout << "Thread 3 executing\n";
            ++n;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    int n = 0;
};

class baz {
   public:
    void operator()() {
        for (int i = 0; i < 5; ++i) {
            std::cout << "Thread 4 executing\n";
            ++n;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    int n = 0;
};

int main() {
    threadpp::ThreadPool pool(4);
    int n = 0;
    foo f;
    baz b;
    pool.Add(f1, n + 1);        // pass by value
    pool.Add(f2, std::ref(n));  // pass by reference
    pool.Add(&foo::bar, &f);    // runs foo::bar() on object f
    pool.Add(b);                // runs baz::operator() on a copy of object b
    pool.Join();
    std::cout << "Final value of n is " << n << '\n';
    std::cout << "Final value of f.n (foo::n) is " << f.n << '\n';
    std::cout << "Final value of b.n (baz::n) is " << b.n << '\n';
}
