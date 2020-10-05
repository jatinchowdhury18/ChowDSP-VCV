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

    onSampleRateChange();

    nrSolver.f_NL = [=] (float x) -> float {
        return std::tanh(x) / params[FB_DRIVE_PARAM].getValue();
    };

    nrSolver.f_NL_prime = [=] (float x) -> float {
        const auto coshX = std::cosh(x);
        return 1.0f / (coshX * coshX);
    };
}

void WarpFilter::onSampleRateChange() {
    float newSampleRate = getSampleRate();
    oversample.reset(newSampleRate);
}

void WarpFilter::process(const ProcessArgs& args) {
    auto freq = pow(highFreq / lowFreq, params[FREQ_PARAM].getValue()) * lowFreq;
    auto q = pow(qBase, params[Q_PARAM].getValue()) * qMult + qOff;
    auto gain = pow(10.0f, params[GAIN_PARAM].getValue() / 20.0f);

    filter.setParameters(BiquadFilter::PEAK, freq / (OSRatio * args.sampleRate), q, gain);
    filter.setDrive(params[DRIVE_PARAM].getValue());

    nrSolver.driveParam = params[FB_DRIVE_PARAM].getValue();
    nrSolver.fbParam = params[FB_PARAM].getValue();

    float x = inputs[AUDIO_IN].getVoltage();
    
    oversample.upsample(x);
    for(int k = 0; k < OSRatio; k++)
        oversample.osBuffer[k] = processOS(oversample.osBuffer[k]);
    float y = oversample.downsample();

    outputs[AUDIO_OUT].setVoltage(y);
}

// oversampled process
float WarpFilter::processOS(float x) {
    return nrSolver.process(x, filter.b[0], filter.z[1], [=] (float x) { filter.process(x); });
}
