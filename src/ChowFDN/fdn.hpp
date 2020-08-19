#ifndef FDN_H_INCLUDED
#define FDN_H_INCLUDED

#include "../plugin.hpp"
#include "mixing_matrix_utils.hpp"
#include "../shared/delay_line.hpp"
#include "../shared/shelf_filter.hpp"
#include <numeric>

template<size_t N>
class FDN {
public:
    FDN();

    void reset();

    // T60 times in seconds, size from 0 to 1
    void prepare(const Module::ProcessArgs& args, float size, float t60Low, float t60High, int curDelays);

    inline float process(float x, int curDelays) {
        float y = 0.0f;

        // accumulate from delay lines
        for(int sumInd = 0; sumInd < curDelays; ++sumInd)
            delayReads[sumInd] = delayLines[sumInd].popSample();

        for(int dInd = 0; dInd < curDelays; ++dInd) {
            // multiply by mixing matrix
            auto accum = std::inner_product(matrix.matrix[dInd], matrix.matrix[dInd] + curDelays, delayReads, 0.0f);

            y += accum; // add to output
            accum += x; // add input to accumulator
            accum = shelfs[dInd].process(accum);
            delayLines[dInd].pushSample(accum);
            delayLines[dInd].updateReadPointer();
        }

        return y;
    }

private:
    DelayLine<float, DelayLineInterpolationTypes::Lagrange3rd> delayLines[N];
    std::vector<int> delayLensMs;
    float curDelaySamples[N];
    
    ShelfFilter shelfs[N];
    float gLow[N];
    float gHigh[N];

    Matrix<N> matrix;
    float delayReads[N];

    float oldSize = -1.0f;
    float oldT60Low = -1.0f;
    float oldT60High = -1.0f;
    int oldCurDelays = -1;
};

#endif // FDN_H_INCLUDED
