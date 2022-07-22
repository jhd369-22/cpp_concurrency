#include <condition_variable>
#include <functional>
#include <mutex>
#include <ra/queue.hpp>
#include <thread>

#define MAX_QUEUE 32  // Maximum number of elements in the queue, at least 32.

namespace ra::concurrency {

    // Thread pool class.
    class thread_pool {
        public:
            // An unsigned integral type used to represent sizes.
            using size_type = std::size_t;

            // Creates a thread pool with the number of threads equal to the
            // hardware concurrency level (if known); otherwise the number of
            // threads is set to 2.
            thread_pool();

            // Creates a thread pool with num_threads threads.
            // Precondition: num_threads > 0
            thread_pool(std::size_t num_threads);

            // A thread pool is not copyable or movable.
            thread_pool(const thread_pool&) = delete;
            thread_pool& operator=(const thread_pool&) = delete;
            thread_pool(thread_pool&&) = delete;
            thread_pool& operator=(thread_pool&&) = delete;

            // Destroys a thread pool, shutting down the thread pool first
            // (if not already shutdown).
            ~thread_pool();

            // Gets the number of threads in the thread pool.
            // This function is not thread safe.
            size_type size() const;

            // Enqueues a task for execution by the thread pool.
            // This function inserts the task specified by the callable
            // entity func into the queue of tasks associated with the
            // thread pool.
            // This function may block if the number of currently
            // queued tasks is sufficiently large.
            // Note: The rvalue reference parameter is intentional and
            // implies that the schedule function is permitted to change
            // the value of func (e.g., by moving from func).
            // Precondition: The thread pool is not in the shutdown state
            // and is not currently in the process of being shutdown via
            // the shutdown member function.
            // This function is thread safe.
            void schedule(std::function<void()>&& func);

            // Shuts down the thread pool.
            // This function places the thread pool into a state where
            // new tasks will no longer be accepted via the schedule
            // member function.
            // Then, the function blocks until all queued tasks
            // have been executed and all threads in the thread pool
            // are idle (i.e., not currently executing a task).
            // Finally, the thread pool is placed in the shutdown state.
            // If the thread pool is already shutdown at the time that this
            // function is called, this function has no effect.
            // After the thread pool is shutdown, it can only be destroyed.
            // This function is thread safe.
            void shutdown();

            // Tests if the thread pool has been shutdown.
            // This function is not thread safe.
            bool is_shutdown() const;

        private:
            // Size of the thread pool.
            size_type num_threads_;

            // Number of threads are idle.
            size_type idle_threads_;

            // A queue of tasks.
            queue<std::function<void()>> tasks_;

            // A mutex used to protect the queue of tasks.
            mutable std::mutex mutex_;

            // A condition variable used to signal idle threads.
            mutable std::condition_variable condition_pop_;

            // A condition variable used to signal tasks to be scheduled.
            mutable std::condition_variable condition_push_;

            // A condition variable used to signal shutdown of the thread pool.
            mutable std::condition_variable condition_shutdown_;

            // A vector of threads.
            std::vector<std::thread> threads_;

            // A flag indicating whether the thread pool has been shutdown.
            bool shutdown_;
    };
}  // namespace ra::concurrency