#pragma once

#include <cmath>

/**
 * Class to process a filter with a delay-free loop
 */
template<size_t MAX_ITER=4>
struct DFLFilter {
    float driveParam = 1.0f;
    float fbParam = 0.0f;
    float y1 = 0.0f;

    void reset() { y1 = 0.0f; }

    inline std::pair<float, float> process(float x, float b, float z) {
        for (size_t k = 0; k < MAX_ITER; ++k) {
            const auto yDrive = y1 * driveParam;
            const auto delta = -1.0f * func(x, y1, yDrive, z, b, fbParam)
                    / func_deriv (yDrive, b, fbParam);

            y1 += delta;
        }

        float y0 = x + fbParam * f_NL(y1 * driveParam);
        return std::make_pair(y0, y1);
    }

    inline float func (float x, float y, float yDrive, float Hn, float h0, float G) const noexcept
    {
        return y - h0 * (x + G * f_NL(yDrive)) - Hn;
    }

    inline float func_deriv(float yDrive, float h0, float G) const noexcept
    {
        return 1.0f - h0 * G * f_NL_prime(yDrive);
    }

    inline float f_NL(float x) const noexcept {
        return std::tanh(x) / driveParam;
    };

    inline float f_NL_prime(float x) const noexcept {
        const auto coshX = std::cosh(x);
        return 1.0f / (coshX * coshX);
    };
};
