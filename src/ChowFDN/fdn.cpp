#include "fdn.hpp"
#include "../shared/delay_utils.hpp"

template<size_t N>
FDN<N>::FDN()
{
    // allocate memory here
    delayLensMs = DelayUtils::generateDelayLengths (N, 45, 1.1f, NextDelayType::NextPrime);

    // rearrange so the shortest delays aren't all first
    for(size_t i = 0; i < N / 2; ++i) {
        std::swap(delayLensMs[i], delayLensMs[N - i - 1]);
    }

    std::fill(gLow, &gLow[N], 0.0f);
    std::fill(gHigh, &gHigh[N], 0.0f);
    std::fill(curDelaySamples, &curDelaySamples[N], 1.0f);

    MixingMatrixUtils::orthonormal(matrix);
}

template<size_t N>
void FDN<N>::reset() {
    for(size_t dInd = 0; dInd < N; ++dInd)
    {
        delayLines[dInd].reset();
        shelfs[dInd].reset();
    }
}

template<size_t N>
void FDN<N>::prepare(const Module::ProcessArgs& args, float size, float t60Low, float t60High, int curDelays) {
    const auto fs = args.sampleRate;

    bool needsSizeUpdate = (size != oldSize) || (curDelays != oldCurDelays);
    bool needsShelfUpdate = (t60Low != oldT60Low) || (t60High != oldT60High) || needsSizeUpdate;

    if(! needsShelfUpdate)
        return;

    if(needsSizeUpdate) {
        // compute delay line lengths
        for(int dInd = 0; dInd < curDelays; ++dInd) {
            const auto curDelayLenMs = (float) delayLensMs[dInd] * size;
            curDelaySamples[dInd] = (curDelayLenMs / 1000.0f) * fs;
            delayLines[dInd].setDelay (curDelaySamples[dInd]);
        }

        oldSize = size;
    }

    if(t60Low != oldT60Low || needsSizeUpdate) {
        for(int dInd = 0; dInd < curDelays; ++dInd)
            gLow[dInd] = DelayUtils::calcGainForT60(curDelaySamples[dInd], fs, t60Low);

        oldT60Low = t60Low;
    }

    if(t60High != oldT60High || needsSizeUpdate) {
        for(int dInd = 0; dInd < curDelays; ++dInd)
            gHigh[dInd] = DelayUtils::calcGainForT60(curDelaySamples[dInd], fs, t60High);

        oldT60High = t60High;
    }

    if(needsShelfUpdate) {
        for(int dInd = 0; dInd < curDelays; ++dInd)
            shelfs[dInd].calcCoefs(clamp(gLow[dInd], -1.0f, 1.0f), clamp(gHigh[dInd], -1.0f, 1.0f), 800.0f, fs);
    }

    oldCurDelays = curDelays;
}

template class FDN<4>;
template class FDN<8>;
template class FDN<16>;
