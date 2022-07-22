#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <ra/thread_pool.hpp>

TEST_CASE("counter increment", "[thread_pool]") {
    namespace rc = ra::concurrency;
    rc::thread_pool::size_type counter = 0;
    rc::thread_pool pool(10);

    for (int i = 0; i < 100; ++i) {
        pool.schedule(std::function<void()>([&counter]() {
            ++counter;
        }));
    }

    CHECK(pool.size() == 10);
    pool.shutdown();
    CHECK(pool.is_shutdown() == true);
    CHECK(counter == 100);
}