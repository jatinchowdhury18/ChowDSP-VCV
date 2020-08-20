#include "modulebench.hpp"
#include <random>

namespace {
    constexpr float fs = 44100.0f;

    enum {
        NUM_SECONDS = 5,
        NUM_SAMPLES = (int) fs * NUM_SECONDS,
    };
}

ModuleBench::ModuleBench (rack::plugin::Model* model, ParamVec params) {
    module.reset(model->createModule());
    args.sampleRate = fs;
    args.sampleTime = 1.0f / fs;

    generateBuffers();

    setParams (params);

    benchFunc = [=] { process(); };
}

ModuleBench::~ModuleBench() {
    module.reset();
}

double ModuleBench::getNumSeconds() const noexcept { return (double) NUM_SECONDS; }

void ModuleBench::setParams (ParamVec params) {
    for(size_t i = 0; i < params.size(); ++i)
        module->params[params[i].first].setValue (params[i].second);
}

void ModuleBench::generateBuffers() {
    const int numChannels = module->inputs.size();

    int seed = 0x1234;
    std::default_random_engine generator(seed);
    std::uniform_real_distribution<float> dist(-5.0f, 5.0f);

    inputs.clear();
    for(int ch = 0; ch < numChannels; ++ch) {
        std::vector<float> channel (NUM_SAMPLES);
        for(int i = 0; i < NUM_SAMPLES; ++i)
            channel[i] = dist(generator);

        inputs.push_back(channel);
    }
}

void ModuleBench::process() {
    for(int i = 0; i < NUM_SAMPLES; ++i) {
        for(size_t ch = 0; ch < module->inputs.size(); ++ch)
            module->inputs[ch].setVoltage(inputs[ch][i]);

        module->process(args);
    }
}
