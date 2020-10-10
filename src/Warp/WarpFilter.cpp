#include "WarpFilter.hpp"

namespace {
    constexpr float highFreq = 1000.0f;
    constexpr float lowFreq = 5.0f;
    constexpr float qMult = 0.25f;
    constexpr float qBase = 19.75f;
    constexpr float qOff = 0.05f;
}

WarpFilter::WarpFilter() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(FREQ_PARAM, 0.0f, 1.0f, 0.5f, "Freq.", " Hz", highFreq / lowFreq, lowFreq);
    configParam(Q_PARAM, 0.15f, 1.0f, 0.324f, "Q", "", qBase, qMult, qOff);
    configParam(GAIN_PARAM, -24.0f, 24.0f, -6.0f, "Gain", " dB");
    configParam(DRIVE_PARAM, 1.0f, 10.0f, 1.0f, "Drive");
    configParam(FB_DRIVE_PARAM, 1.0f, 10.0f, 1.0f, "FB Drive");
    configParam(FB_PARAM, 0.0f, 0.9f, 0.0f, "Feedback");

    oversample.setOversamplingIndex(1); // default 2x oversampling
    onSampleRateChange();
}

void WarpFilter::onSampleRateChange() {
    float newSampleRate = getSampleRate();
    oversample.reset(newSampleRate);
    cookParams(newSampleRate);
}

void WarpFilter::cookParams(float sampleRate) noexcept {
    auto freq = pow(highFreq / lowFreq, params[FREQ_PARAM].getValue()) * lowFreq;
    auto q = pow(qBase, params[Q_PARAM].getValue()) * qMult + qOff;
    auto gain = pow(10.0f, params[GAIN_PARAM].getValue() / 20.0f);

    filter.setParameters(BiquadFilter::PEAK, freq / (oversample.getOversamplingRatio() * sampleRate), q, gain);
    filter.setDrive(params[DRIVE_PARAM].getValue());

    nrSolver.driveParam = params[FB_DRIVE_PARAM].getValue();
    nrSolver.fbParam = params[FB_PARAM].getValue();
}

void WarpFilter::process(const ProcessArgs& args) {
    float x = inputs[AUDIO_IN].getVoltage();
    
    oversample.upsample(x);
    float* osBuffer = oversample.getOSBuffer();
    for(int k = 0; k < oversample.getOversamplingRatio(); k++)
        osBuffer[k] = processOS(osBuffer[k]);
    float y = oversample.downsample();

    outputs[AUDIO_OUT].setVoltage(y);
}

// oversampled process
float WarpFilter::processOS(float x) noexcept {
    auto y0_y1 = nrSolver.process(x, filter.b[0], filter.z[1]);
    filter.process(y0_y1.first);
    return y0_y1.second;
}
