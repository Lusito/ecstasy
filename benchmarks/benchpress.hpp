/*
* Copyright (C) 2015 Christopher Gilbert.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#ifndef BENCHPRESS_HPP
#define BENCHPRESS_HPP

#include <algorithm>   // max, min
#include <atomic>      // atomic_intmax_t
#include <chrono>      // high_resolution_timer, duration
#include <functional>  // function
#include <iomanip>     // setw
#include <iostream>    // cout
#include <regex>       // regex, regex_match
#include <sstream>     // stringstream
#include <string>      // string
#include <thread>      // thread
#include <vector>      // vector

namespace benchpress {

/*
 * The options class encapsulates all options for running benchmarks.
 *
 * When including benchpress, a main function can be emitted which includes a command-line parser for building an
 * options object. However from time-to-time it may be necessary for the developer to have to build their own main
 * stub and construct the options object manually.
 *
 * options opts;
 * opts
 *     .bench(".*")
 *     .benchtime(1)
 *     .cpu(4);
 */
class options {
    std::string d_bench;
    size_t      d_benchtime;
    size_t      d_cpu;
public:
    options()
        : d_bench(".*")
        , d_benchtime(1)
        , d_cpu(std::thread::hardware_concurrency())
    {}
    options& bench(const std::string& bench) {
        d_bench = bench;
        return *this;
    }
    options& benchtime(size_t benchtime) {
        d_benchtime = benchtime;
        return *this;
    }
    options& cpu(size_t cpu) {
        d_cpu = cpu;
        return *this;
    }
    std::string get_bench() const {
        return d_bench;
    }
    size_t get_benchtime() const {
        return d_benchtime;
    }
    size_t get_cpu() const {
        return d_cpu;
    }
};

class context;

class auto_register_base {
    std::string                   d_name;
	
public:
	auto_register_base(const std::string& name)
        : d_name(name) {}
		
	virtual void init(context *ctx) = 0;
	virtual void run(context *ctx) = 0;
	virtual void shutdown(context *ctx) = 0;
	
    std::string                   get_name() const { return d_name; }
};

/*
 * The registration class is responsible for providing a single global point of reference for registering
 * benchmark functions.
 *
 * registration::get_ptr()->register_benchmark(info);
 */
class registration {
    static registration*        d_this;
    std::vector<auto_register_base *> d_benchmarks;

public:
    static registration* get_ptr() {
        if (nullptr == d_this) {
            d_this = new registration();
        }
        return d_this;
    }

    void register_benchmark(auto_register_base* benchmark) {
        d_benchmarks.push_back(benchmark);
    }

    const std::vector<auto_register_base *> &get_benchmarks() { return d_benchmarks; }
};

/*
 * The auto_register class is a helper used to register benchmarks.
 */
template<typename T>
class auto_register : public auto_register_base {
public:
    auto_register(const std::string& name) : auto_register_base(name) {
        registration::get_ptr()->register_benchmark(this);
    }
	
	void init(context *ctx) override {
		instance = new T();
	}
	
	void run(context *ctx) override {
		instance->run(ctx);
	}
	
	void shutdown(context *ctx) override {
		delete instance;
		instance = nullptr;
	}
	
private:
	T *instance = nullptr;
};

#define CONCAT(x, y) x ## y
#define CONCAT2(x, y) CONCAT(x, y)

// The BENCHMARK macro is a helper for creating benchmark functions and automatically registering them with the
// registration class.
#define BENCHMARK(c, s) benchpress::auto_register<c> CONCAT2(register_, __LINE__)((s));

// This macro will prevent the compiler from removing a redundant code path which has no side-effects.
#define DISABLE_REDUNDANT_CODE_OPT() { asm(""); }

/*
 * The result class is responsible for producing a printable string representation of a benchmark run.
 */
class result {
    size_t                   d_num_iterations;
    std::chrono::nanoseconds d_duration;
    size_t                   d_num_bytes;

public:
    result(size_t num_iterations, std::chrono::nanoseconds duration, size_t num_bytes)
        : d_num_iterations(num_iterations)
        , d_duration(duration)
        , d_num_bytes(num_bytes)
    {}

    size_t get_ns_per_op() const {
        if (d_num_iterations <= 0) {
            return 0;
        }
        return d_duration.count() / d_num_iterations;
    }

    double get_mb_per_s() const {
        if (d_num_iterations <= 0 || d_duration.count() <= 0 || d_num_bytes <= 0) {
            return 0;
        }
        return ((double(d_num_bytes) * double(d_num_iterations) / double(1e6)) /
                double(std::chrono::duration_cast<std::chrono::seconds>(d_duration).count()));
    }

    std::string to_string() const {
        std::stringstream tmp;
        tmp << std::setw(12) << std::right << d_num_iterations;
        size_t npo = get_ns_per_op();
        tmp << std::setw(12) << std::right << npo << std::setw(0) << " ns/op";
        double mbs = get_mb_per_s();
        if (mbs > 0.0) {
            tmp << std::setw(12) << std::right << mbs << std::setw(0) << " MB/s";
        }
        return std::string(tmp.str());
    }
};

/*
 * The parallel_context class is responsible for providing a thread-safe context for parallel benchmark code.
 */
class parallel_context {
    std::atomic_intmax_t d_num_iterations;
public:
    parallel_context(size_t num_iterations)
        : d_num_iterations(num_iterations)
    {}

    bool next() {
        return (d_num_iterations.fetch_sub(1) > 0);
    }
};

/*
 * The context class is responsible for providing an interface for capturing benchmark metrics to benchmark functions.
 */
class context {
    bool                                           d_timer_on;
    std::chrono::high_resolution_clock::time_point d_start;
    std::chrono::nanoseconds                       d_duration;
    std::chrono::seconds                           d_benchtime;
    size_t                                         d_num_iterations;
    size_t                                         d_num_threads;
    size_t                                         d_num_bytes;
    auto_register_base *                           d_benchmark;

public:
    context(auto_register_base *benchmark, const options& opts)
        : d_timer_on(false)
        , d_start()
        , d_duration()
        , d_benchtime(std::chrono::seconds(opts.get_benchtime()))
        , d_num_iterations(1)
        , d_num_threads(opts.get_cpu())
        , d_num_bytes(0)
        , d_benchmark(benchmark)
    {}

    size_t num_iterations() const { return d_num_iterations; }

    void set_num_threads(size_t n) { d_num_threads = n; }
    size_t num_threads() const { return d_num_threads; }

    void start_timer() {
        if (!d_timer_on) {
            d_start = std::chrono::high_resolution_clock::now();
            d_timer_on = true;
        }
    }
    void stop_timer() {
        if (d_timer_on) {
            d_duration += std::chrono::high_resolution_clock::now() - d_start;
            d_timer_on = false;
        }
    }
    void reset_timer() {
        if (d_timer_on) {
            d_start = std::chrono::high_resolution_clock::now();
        }
        d_duration = std::chrono::nanoseconds::zero();
    }

    void set_bytes(int64_t bytes) { d_num_bytes = bytes; }

    size_t get_ns_per_op() {
        if (d_num_iterations <= 0) {
            return 0;
        }
        return d_duration.count() / d_num_iterations;
    }

    void run_n(size_t n) {
        d_num_iterations = n;
        d_benchmark->init(this);
        reset_timer();
        start_timer();
        d_benchmark->run(this);
        stop_timer();
        d_benchmark->shutdown(this);
    }

    void run_parallel(std::function<void(parallel_context*)> f) {
        parallel_context pc(d_num_iterations);
        std::vector<std::thread> threads;
        for (size_t i = 0; i < d_num_threads; ++i) {
            threads.push_back(std::thread([&pc,&f]() -> void {
                f(&pc);
            }));
        }
        for(auto& thread : threads){
            thread.join();
        }
    }

    result run() {
        size_t n = 1;
        run_n(n);
        while (d_duration < d_benchtime && n < 1e9) {
            size_t last = n;
            if (get_ns_per_op() == 0) {
                n = 1e9;
            } else {
                n = d_duration.count() / get_ns_per_op();
            }
            n = std::max(std::min(n+n/2, 100*last), last+1);
            n = round_up(n);
            run_n(n);
        }
        return result(n, d_duration, d_num_bytes);
    }

private:
    template<typename T>
    T round_down_10(T n) {
        int tens = 0;
        while (n > 10) {
            n /= 10;
            tens++;
        }
        int result = 1;
        for (int i = 0; i < tens; ++i) {
            result *= 10;
        }
        return result;
    }

    template<typename T>
    T round_up(T n) {
        T base = round_down_10(n);
        if (n < (2 * base)) {
            return 2 * base;
        }
        if (n < (5 * base)) {
            return 5 * base;
        }
        return 10 * base;
    }
};

#ifdef BENCHPRESS_CONFIG_MAIN
/*
 * The run_benchmarks function will run the registered benchmarks.
 */
void run_benchmarks(const options& opts) {
    std::regex match_r(opts.get_bench());
    auto benchmarks = registration::get_ptr()->get_benchmarks();
    for (auto benchmark : benchmarks) {
        if (std::regex_match(benchmark->get_name(), match_r)) {
            context c(benchmark, opts);
            auto r = c.run();
            std::cout << std::setw(35) << std::left << benchmark->get_name() << r.to_string() << std::endl;
        }
    }
}
#endif

template < typename T > std::string to_string( const T& n )
{
	std::ostringstream stm ;
	stm << n ;
	return stm.str() ;
}
} // namespace benchpress

/*
 * If BENCHPRESS_CONFIG_MAIN is defined when the file is included then a main function will be emitted which provides a
 * command-line parser and then executes run_benchmarks.
 */
#ifdef BENCHPRESS_CONFIG_MAIN

benchpress::registration* benchpress::registration::d_this;
int main(int argc, char** argv) {
    std::chrono::high_resolution_clock::time_point bp_start = std::chrono::high_resolution_clock::now();
    benchpress::options bench_opts;
    benchpress::run_benchmarks(bench_opts);
    float duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - bp_start
    ).count() / 1000.f;
    std::cout << argv[0] << " " << duration << "s" << std::endl;
    return 0;
}
#endif

#endif // BENCHPRESS_HPP