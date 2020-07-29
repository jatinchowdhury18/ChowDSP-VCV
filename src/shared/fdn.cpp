#include "fdn.hpp"
#include "delay_utils.hpp"

FDN::FDN(int numDelays) :
    numDelays (numDelays),
    matrix (numDelays)
{
    // allocate memory here
    delayLensMs = DelayUtils::generateDelayLengths (numDelays, 45, 1.1f, NextDelayType::AddOne);
    delayLines = new Delay[numDelays];
    shelfs = new ShelfFilter[numDelays];
    delayReads = new float[numDelays];

    MixingMatrixUtils::orthonormal(matrix);
}

FDN::~FDN()
{
    // NO MEMORY LEAKS
    delete[] delayReads;
    delete[] delayLines;
    delete[] shelfs;
}

void FDN::reset() {
    for (int dInd = 0; dInd < numDelays; ++dInd)
    {
        delayLines[dInd].reset();
        shelfs[dInd].reset();
    }
}

void FDN::prepare(const Module::ProcessArgs& args, float size, float t60Low, float t60High) {
    const auto fs = args.sampleRate;

    for (int dInd = 0; dInd < numDelays; ++dInd) {
        // compute delay line lengths
        const auto curDelayLen = (float) delayLensMs[dInd] * size;
        delayLines[dInd].setProcessArgs(args);
        delayLines[dInd].setDelayTimeMs(curDelayLen);

        const auto curDelaySamples = (curDelayLen / 1000.0f) * fs;
        const auto gLow = DelayUtils::calcGainForT60(curDelaySamples, fs, t60Low);
        const auto gHigh = DelayUtils::calcGainForT60(curDelaySamples, fs, t60High);

        shelfs[dInd].calcCoefs(gLow, gHigh, 2500.0f, fs);
    }
}
