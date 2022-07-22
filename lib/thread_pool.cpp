#include <ra/thread_pool.hpp>

namespace ra::concurrency {

    thread_pool::thread_pool() : threads_(), tasks_(MAX_QUEUE), idle_threads_(0), shutdown_(false) {
        num_threads_ = std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() : 2;

        for (size_type i = 0; i < num_threads_; ++i) {
            threads_.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mutex_);

                        ++idle_threads_;

                        // To notify the shutdown function that the idle thread count has increased.
                        if (tasks_.is_closed()) {
                            condition_shutdown_.notify_one();
                        }

                        condition_pop_.wait(lock, [this]() {
                            return !tasks_.is_empty() || shutdown_;
                        });

                        if (shutdown_) {
                            return;
                        }

                        --idle_threads_;

                        tasks_.pop(task);
                        condition_push_.notify_one();
                    }
                    task();
                }
            });
        }
    }

    thread_pool::thread_pool(std::size_t num_threads) : threads_(), num_threads_(num_threads), tasks_(MAX_QUEUE), idle_threads_(0), shutdown_(false) {
        for (size_type i = 0; i < num_threads_; ++i) {
            threads_.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mutex_);

                        ++idle_threads_;

                        // To notify the shutdown function that the idle thread count has increased.
                        if (tasks_.is_closed()) {
                            condition_shutdown_.notify_one();
                        }

                        condition_pop_.wait(lock, [this]() {
                            return !tasks_.is_empty() || shutdown_;
                        });

                        if (shutdown_) {
                            return;
                        }

                        --idle_threads_;

                        tasks_.pop(task);
                        condition_push_.notify_one();
                    }
                    task();
                }
            });
        }
    }

    thread_pool::~thread_pool() {
        if (!shutdown_) {
            shutdown();
        }

        for (auto& thread : threads_) {
            thread.join();
        }
    }

    thread_pool::size_type thread_pool::size() const {
        return num_threads_;
    }

    void thread_pool::schedule(std::function<void()>&& func) {
        std::unique_lock<std::mutex> lock(mutex_);

        condition_push_.wait(lock, [this]() {
            return !tasks_.is_full() || shutdown_;
        });

        if (shutdown_ || tasks_.is_closed()) {
            return;
        }

        tasks_.push(std::move(func));

        condition_pop_.notify_one();
    }

    void thread_pool::shutdown() {
        std::unique_lock<std::mutex> lock(mutex_);

        if (shutdown_) {
            return;
        }

        tasks_.close();
        condition_shutdown_.wait(lock, [this]() { return tasks_.is_empty() && (idle_threads_ == num_threads_); });
        shutdown_ = true;

        // Notify all threads that the thread pool is shutting down, so that they can exit.
        condition_push_.notify_all();
        condition_pop_.notify_all();
    }

    bool thread_pool::is_shutdown() const {
        return shutdown_;
    }
}  // namespace ra::concurrency
