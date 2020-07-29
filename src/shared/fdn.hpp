#ifndef FDN_H_INCLUDED
#define FDN_H_INCLUDED

#include "plugin.hpp"
#include "mixing_matrix_utils.hpp"
#include "delay.hpp"
#include "shelf_filter.hpp"
#include <numeric>

class FDN {
public:
    FDN (int numDelays);
    ~FDN();

    void reset();

    // T60 times in seconds, size from 0 to 1
    void prepare(const Module::ProcessArgs& args, float size, float t60Low, float t60High);

    inline float process(float x) {
        float y = 0.0f;

        // accumulate from delay lines
        for (int sumInd = 0; sumInd < numDelays; ++sumInd)
            delayReads[sumInd] = delayLines[sumInd].read();

        for (int dInd = 0; dInd < numDelays; ++dInd) {
            // multiply by mixing matrix
            auto accum = std::inner_product(matrix.matrix[dInd], matrix.matrix[dInd] + numDelays, delayReads, 0.0f);

            y += accum; // add to output
            accum += x; // add input to accumulator
            accum = shelfs[dInd].process(accum);
            delayLines[dInd].write(accum);
        }

        return y;
    }

private:
    const int numDelays;

    std::vector<int> delayLensMs;

    Delay* delayLines = nullptr;
    ShelfFilter* shelfs = nullptr;

    Matrix matrix;
    float* delayReads = nullptr;
};

#endif // FDN_H_INCLUDED
