#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <ra/thread_pool.hpp>
// #include <thread> // already included in thread_pool.hpp
// #include <vector> // already included in thread_pool.hpp

TEST_CASE("counter increment", "[thread_pool]") {
    namespace rc = ra::concurrency;
    rc::thread_pool::size_type counter = 0;
    rc::thread_pool pool(10);
    std::vector<std::thread> threads;
    std::mutex m;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(std::thread([&]() {
            for (int i = 0; i < 1000; ++i) {
                pool.schedule(std::function<void()>([&]() {
                    std::scoped_lock<std::mutex> lock(m);
                    ++counter;
                }));
            }
        }));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    CHECK(pool.size() == 10);
    pool.shutdown();
    CHECK(pool.is_shutdown() == true);
    CHECK(counter == 10000);
}