#include <chrono>
#include <ra/julia_set.hpp>
#include <string_view>

// a template function to show the data type of the template parameter
template <typename T>
constexpr auto type_name() {
    std::string_view name, prefix, suffix;
#ifdef __clang__
    name = __PRETTY_FUNCTION__;
    prefix = "auto type_name() [T = ";
    suffix = "]";
#elif defined(__GNUC__)
    name = __PRETTY_FUNCTION__;
    prefix = "constexpr auto type_name() [with T = ";
    suffix = "]";
#elif defined(_MSC_VER)
    name = __FUNCSIG__;
    prefix = "auto __cdecl type_name<";
    suffix = ">(void)";
#endif
    name.remove_prefix(prefix.size());
    name.remove_suffix(suffix.size());
    return name;
}

template <typename T>
void test(){
    int height = 512;
    int width = 512;
    int max_iters = 255;
    std::complex<T> bottom_left(-1.25, -1.25);
    std::complex<T> top_right(1.25, 1.25);
    std::complex<T> c(0.37, -0.16);

    // print the type of T
    std::cout << "Data type: " << type_name<decltype(T(1))>() << "\n";
    std::cout << "Height: " << height << "\n";
    std::cout << "Width: " << width << "\n";
    std::cout << "Max iterations: " << max_iters << "\n";

    // 1 thread
    boost::multi_array<int, 2> a(boost::extents[height][width]);
    
    auto start = std::chrono::steady_clock::now();
    ra::fractal::compute_julia_set<T>(bottom_left, top_right, c, max_iters, a, 1);
    auto end = std::chrono::steady_clock::now();

    // counts the duration of the computation
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "number of threads: 1, time: " << duration.count() << " ms" << "\n";

    // 2 threads
    boost::multi_array<int, 2> b(boost::extents[height][width]);

    start = std::chrono::steady_clock::now();
    ra::fractal::compute_julia_set<T>(bottom_left, top_right, c, max_iters, b, 2);
    end = std::chrono::steady_clock::now();

    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "number of threads: 2, time: " << duration.count() << " ms" << "\n";

    // 4 threads
    boost::multi_array<int, 2> d(boost::extents[height][width]);

    start = std::chrono::steady_clock::now();
    ra::fractal::compute_julia_set<T>(bottom_left, top_right, c, max_iters, d, 4);
    end = std::chrono::steady_clock::now();

    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "number of threads: 4, time: " << duration.count() << " ms" << "\n";

    // 8 threads
    boost::multi_array<int, 2> e(boost::extents[height][width]);

    start = std::chrono::steady_clock::now();
    ra::fractal::compute_julia_set<T>(bottom_left, top_right, c, max_iters, e, 8);
    end = std::chrono::steady_clock::now();

    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "number of threads: 8, time: " << duration.count() << " ms" << "\n\n";
}

int main(){
    test<float>();
    test<double>();
    test<long double>();

    return 0;
}
