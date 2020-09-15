#pragma once

#include <functional>
#include <chrono>
#include <thread>

using namespace std::chrono;

class Benchmark {
public:
    Benchmark() = default;

    std::function<void()> benchFunc = [] { std::this_thread::sleep_for(milliseconds(100)); };

    double run() {
        double timeSum = 0;

        for(int i = 0; i < NUM_ITER; ++i) {
            auto start = high_resolution_clock::now();
            
            // RUN FUNCTION HERE
            benchFunc();

            auto end = high_resolution_clock::now();
            auto dur = duration_cast<duration<double>>(end - start);
            timeSum += dur.count();
        }

        return timeSum / (double) NUM_ITER;
    }

private:
    enum {
        NUM_ITER = 50,
    };
};
