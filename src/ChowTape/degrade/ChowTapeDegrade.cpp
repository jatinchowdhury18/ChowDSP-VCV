#include "../../plugin.hpp"
#include "DegradeFilter.h"
#include "DegradeNoise.h"
#include "../../shared/LevelDetector.hpp"

struct ChowTapeDegrade : Module {
    enum ParamIds {
        DEPTH_PARAM,
        AMOUNT_PARAM,
        VAR_PARAM,
        ENV_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        AUDIO_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        AUDIO_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    ChowTapeDegrade() {
        config(NUM_PARAMS, NUM_INPUTS,NUM_OUTPUTS, NUM_LIGHTS);

        configInput(AUDIO_INPUT, "Audio");
        configOutput(AUDIO_OUTPUT, "Audio");
        configBypass(AUDIO_INPUT, AUDIO_OUTPUT);

        configParam(DEPTH_PARAM, 0.0f, 1.0f, 0.0f, "Depth", "%", 0.0f, 100.0f);
        configParam(AMOUNT_PARAM, 0.0f, 1.0f, 0.0f, "Amount", "%", 0.0f, 100.0f);
        configParam(VAR_PARAM, 0.0f, 1.0f, 0.0f, "Variance", "%", 0.0f, 100.0f);
        configParam(ENV_PARAM, 0.0f, 1.0f, 0.0f, "Envelope", "%", 0.0f, 100.0f);

        onSampleRateChange();
        paramDivider.setDivision(ParamDivide);
    }

    void onSampleRateChange() override {
        float sampleRate = getSampleRate();
        noiseProc.prepare(sampleRate);
        filterProc.reset(sampleRate, int(sampleRate * 0.05f));
        levelDetector.prepare(sampleRate);
        gainProc.reset((double) sampleRate, 0.05);
        cookParams(sampleRate);
    }

    void onReset() override {
        Module::onReset();
        onSampleRateChange();
    }

    void cookParams(float fs) {
        const auto depthParam = params[DEPTH_PARAM].getValue();
        const auto amtParam = params[AMOUNT_PARAM].getValue();
        const auto varParam = params[VAR_PARAM].getValue();
        const auto envParam = params[ENV_PARAM].getValue();

        const auto freqHz = 200.0f * std::pow(20000.0f / 200.0f, 1.0f - amtParam);
        const auto gainDB = -24.0f * depthParam;

        noiseProc.setGain(0.33f * depthParam * amtParam);
        filterProc.setFreq(std::min(freqHz + (varParam * (freqHz / 0.6f) * (random::uniform() - 0.5f)), 0.49f * fs));

        const auto envSkew = 1.0f - std::pow(envParam, 0.8f);
        levelDetector.setParameters(10.0f, 20.0f * std::pow(5000.0f / 20.0f, envSkew));
        gainProc.setTargetValue(dsp::dbToAmplitude(std::min(gainDB + (varParam * 36.0f * (random::uniform() - 0.5f)), 3.0f)));
    }

    void process(const ProcessArgs& args) override {
        if(paramDivider.process())
            cookParams(args.sampleRate);

        float x = inputs[AUDIO_INPUT].getVoltage() / 10.0f;
        auto level = levelDetector.processSample(x);

        auto noise = noiseProc.processSample(0.0f);

        if(params[ENV_PARAM].getValue() > 0.0f)
            noise *= level;

        x += noise;
        x = filterProc.processSample(x);
        x *= gainProc.getNextValue();

        outputs[AUDIO_OUTPUT].setVoltage(x * 10.0f);
    }

private:
    enum {
        ParamDivide = 64,
    };

    dsp::ClockDivider paramDivider;

    DegradeFilter filterProc;
    DegradeNoise noiseProc;
    LevelDetector<float> levelDetector;
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> gainProc;
};

struct ChowTapeDegradeWidget : ModuleWidget {
    ChowTapeDegradeWidget(ChowTapeDegrade* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChowTapeDegrade.svg")));
        createScrews(*this);

        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 23.0)), module, ChowTapeDegrade::DEPTH_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 43.0)), module, ChowTapeDegrade::AMOUNT_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 63.0)), module, ChowTapeDegrade::VAR_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 83.0)), module, ChowTapeDegrade::ENV_PARAM));

        addInput(createInputCentered<ChowPort>(mm2px(Vec(15.25, 99.0)), module, ChowTapeDegrade::AUDIO_INPUT));
        addOutput(createOutputCentered<ChowPort>(mm2px(Vec(15.25, 115.0)), module, ChowTapeDegrade::AUDIO_OUTPUT));
    }
};

Model* modelChowTapeDegrade = createModel<ChowTapeDegrade, ChowTapeDegradeWidget>("ChowTapeDegrade");
