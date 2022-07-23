#include <boost/multi_array.hpp>
#include <complex>
#include <iostream>
#include <ra/thread_pool.hpp>

namespace ra::fractal {

    template <typename T>
    int julia_set_point(const std::complex<T>& bottom_left,
                        const std::complex<T>& top_right, const std::complex<T>& c,
                        int max_iters, int height, int width, int x, int y) {
        std::complex<T> z(bottom_left.real() + (T(y) / T(width - 1)) * (top_right.real() - bottom_left.real()),
                          bottom_left.imag() + (T(x) / T(height - 1)) * (top_right.imag() - bottom_left.imag()));
        
        for (int i = 0; i < max_iters; ++i) {
            if (std::abs(z) > 2) {
                return i;
            }
            z = z * z + c;
        }
        // return the smallest value for which |z| > 2 or the max_iters if not exist.
        return max_iters;
    }

    template <class Real>
    void compute_julia_set(const std::complex<Real>& bottom_left,
                           const std::complex<Real>& top_right, const std::complex<Real>& c,
                           int max_iters, boost::multi_array<int, 2>& a, int num_threads) {
        using thread_pool = ra::concurrency::thread_pool;

        int height = a.shape()[0];  // may be const
        int width = a.shape()[1];

        thread_pool tp(num_threads);

        // Enqueue the tasks.
        for (int i = 0; i < height; ++i) {
            tp.schedule(std::move([&, i]() {
                for (int j = 0; j < width; ++j) {
                    a[height - i - 1][j] = julia_set_point<Real>(bottom_left, top_right, c,
                                                                 max_iters, height, width, i, j);
                }
            }));
        }

        // Wait for all tasks to finish.
        tp.shutdown();
    }

    void print_result(const boost::multi_array<int, 2>& a) {
        // Print the result.
        std::cout << "P2 " << a.shape()[1] << " " << a.shape()[0] << " 255"
                  << "\n";
        for (int i = 0; i < a.shape()[0]; ++i) {
            for (int j = 0; j < a.shape()[1]; ++j) {
                std::cout << a[i][j] << ((j == a.shape()[1] - 1) ? "" : " ");
            }
            std::cout << "\n";
        }
    }
}  // namespace ra::fractal