#pragma once

#include <cmath>

/**
 * Class to process a filter with a delay-free loop
 */
template<size_t MAX_ITER=4>
struct DFLFilter {
    float driveParam = 0.5f;
    float fbParam = 0.0f;
    float y1 = 0.0f;

    std::function<float(float)> f_NL = {};
    std::function<float(float)> f_NL_prime = {};

    void reset() { y1 = 0.0f; }

    inline float process(float x, float b, float z, std::function<void(float)> updateState) {
        for (size_t k = 0; k < MAX_ITER; ++k) {
            const auto yDrive = y1* driveParam;
            const auto delta = -1.0f * func (x, y1, yDrive, z, b, fbParam, driveParam)
                    / func_deriv (yDrive, b, fbParam);

            y1 += delta;
        }

        float y0 = x + fbParam * f_NL (y1 * driveParam); //, driveParam);
        updateState (y0);
        return y1;
    }

    inline float func (float x, float y, float yDrive, float Hn, float h0, float G, float drive) const noexcept
    {
        return y - h0 * (x + G * f_NL (yDrive)) - Hn; // , drive)) - Hn;
    }

    inline float func_deriv (float yDrive, float h0, float G) const noexcept
    {
        return 1.0f - h0 * G * f_NL_prime (yDrive);
    }

};
