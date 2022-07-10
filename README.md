# threadpp

threadpp is a lightweight thread pool implementation in C++17 which provides
a task submission interface compatible with `std::thread`'s
[constructor](https://en.cppreference.com/w/cpp/thread/thread/thread).
Tasks are submitted and processed on a first-come, first-served basis, and
their results can be obtained via the `std::future` returned from submission.

## How to Use

To use threadpp in your project, you may either copy the sources under
directories [include](include) and [src](src) to your project, or build it as
a library and link it to your project. To build threadpp, read section "How to
Build".

Below is a simple usage example.

```c++
#include <iostream>
#include <thread_pool.h>

int FortyTwo() { return 42; }

void Foo() {
    ;  // some task
}

int main() {
    // create a ThreadPool with 3 workers
    threadpp::ThreadPool pool(3);

    // submit tasks
    std::future<int> forty_two = pool.Add(FortyTwo);
    std::future<void> foo = pool.Add(Foo);
    std::future<bool> seven_is_even =
        pool.Add([](int x) { return x % 2 == 0; }, 7);

    // get results as needed
    std::cout << forty_two.get() << std::endl;

    // or simply left out the futures and wait for all tasks to finish
    pool.Join();
}
```

More examples are available under the [examples](examples) directory.

## How to Build

To build threadpp, simply run `make`. The compiled library file
`libthreadpp.a` will then be created.

## License

Copyright (c) 2021 - 2022 Zhong Ruoyu. Licensed under
[the MIT License](LICENSE).
