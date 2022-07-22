#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <ra/queue.hpp>
#include <thread>
#include <vector>
#include <cassert>

TEMPLATE_TEST_CASE("single thread basic functionality", "[ra::concurrency::queue]", int, double) {
    namespace ra = ra::concurrency;
    using queue_type = ra::queue<TestType>;

    queue_type q(10);

    CHECK(q.is_empty());
    CHECK(q.max_size() == 10);

    CHECK(q.push(TestType(1)) == queue_type::status::success);
    CHECK(q.push(TestType(2)) == queue_type::status::success);
    CHECK(q.push(TestType(3)) == queue_type::status::success);
    CHECK(q.push(TestType(4)) == queue_type::status::success);
    CHECK(q.push(TestType(5)) == queue_type::status::success);
    CHECK(q.push(TestType(6)) == queue_type::status::success);
    CHECK(q.push(TestType(7)) == queue_type::status::success);
    CHECK(q.push(TestType(8)) == queue_type::status::success);
    CHECK(q.push(TestType(9)) == queue_type::status::success);
    CHECK(q.push(TestType(10)) == queue_type::status::success);

    SECTION("full") {
        CHECK(q.is_full() == true);
    }

    SECTION("closed") {
        q.close();
        CHECK(q.is_closed() == true);
        CHECK(q.push(TestType(11)) == queue_type::status::closed);

        typename queue_type::value_type x;
        CHECK(q.pop(x) == queue_type::status::success);
        CHECK(x == TestType(1));
    }

    SECTION("pop") {
        typename queue_type::value_type x;
        CHECK(q.pop(x) == queue_type::status::success);
        CHECK(x == TestType(1));
    }

    SECTION("clear") {
        q.clear();
        CHECK(q.is_empty() == true);

        q.close();
        typename queue_type::value_type x;
        CHECK(q.pop(x) == queue_type::status::closed);
    }
}

TEMPLATE_TEST_CASE("multi thread basic functionality", "[ra::concurrency::queue]", int, double) {
    namespace ra = ra::concurrency;
    using queue_type = ra::queue<TestType>;

    queue_type q(10);

    SECTION("push and pop without close") {
        std::vector<std::thread> threads;
        for (int i = 0; i < 4; ++i) {
            threads.push_back(std::thread([&q]() {
                for (int j = 0; j < 10; ++j) {
                    typename queue_type::value_type x;
                    assert((q.pop(x) == queue_type::status::success));
                }
            }));

            threads.push_back(std::thread([&q, i]() {
                for (int j = 0; j < 10; ++j) {
                    assert((q.push(TestType(j + i * 10)) == queue_type::status::success));
                }
            }));
        }

        for (auto& t : threads) {
            t.join();
        }

        CHECK(q.is_empty() == true);
        CHECK(q.max_size() == 10);
        CHECK(q.is_full() == false);
    }

    SECTION("push with close") {

        std::thread t1([&q]() {
            for (int j = 0; j < 10; ++j) {
                assert((q.push(TestType(j)) == queue_type::status::success));
            }
        });

        t1.join();

        std::thread t2([&q]() {
            for (int j = 0; j < 10; ++j) {
                assert((q.push(TestType(j)) == queue_type::status::closed));
            }
        });

        q.close();

        t2.join();

        CHECK(q.is_empty() == false);
        CHECK(q.max_size() == 10);
        CHECK(q.is_full() == true);
        CHECK(q.is_closed() == true);
    }

    SECTION("pop with close") {
        std::thread t1([&q]() {
            for (int j = 0; j < 10; ++j) {
                assert((q.push(TestType(j)) == queue_type::status::success));
            }
        });

        t1.join();

        q.close();

        std::thread t2([&q]() {
            for (int j = 0; j < 10; ++j) {
                typename queue_type::value_type x;
                assert((q.pop(x) == queue_type::status::success));
            }
        });


        t2.join();

        CHECK(q.is_empty() == true);
        CHECK(q.max_size() == 10);
        CHECK(q.is_full() == false);
        CHECK(q.is_closed() == true);
    }
}