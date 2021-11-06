#include "../plugin.hpp"
#include "PulseShaper.hpp"

namespace {
    constexpr float widthBase = 300.0f;
    constexpr float widthMult = 1.0f;

    constexpr float decayBase = 30.0f;
    constexpr float decayMult = 10.0f;

    constexpr float capVal = (float) 0.015e-6;
}

struct ChowPulse : Module {
    enum ParamIDs {
        WIDTH_PARAM,
        DECAY_PARAM,
        DOUBLE_PARAM,
        NUM_PARAMS
    };
    enum InputIDs {
        WIDTH_IN,
        DECAY_IN,
        DOUBLE_IN,
        TRIG_IN,
        NUM_INPUTS
    };
    enum OutputIDs {
        ENV_OUT,
        NUM_OUTPUTS
    };
    enum LightIds {
		NUM_LIGHTS
	};

    ChowPulse() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configInput(WIDTH_IN, "Pulse width mod.");
        configInput(DECAY_IN, "Decay mod.");
        configInput(DOUBLE_IN, "Double tap mod.");
        configInput(TRIG_IN, "Trigger");
        configOutput(ENV_OUT, "Envelope");

        configParam(WIDTH_PARAM, 0.0f, 1.0f, 0.5f, "Pulse Width", " ms", widthBase, widthMult);
        configParam(DECAY_PARAM, 0.0f, 1.0f, 0.5f, "Decay Time", " ms", decayBase, decayMult);
        configParam(DOUBLE_PARAM, -1.0f, 1.0f, 0.0f, "Double Tap");

        onSampleRateChange();
        paramDivider.setDivision(ParamDivide);
    }

    void onSampleRateChange() override {
        float newSampleRate = getSampleRate();
        pulseShaper.reset(new PulseShaper(newSampleRate));
    }

    void onReset() override {
        Module::onReset();
        onSampleRateChange();
    }

    inline void cookParams(const ProcessArgs& args) {
        float widthParam = params[WIDTH_PARAM].getValue() + inputs[WIDTH_IN].getVoltage() / 10.0f;
        float pulseWidthMs = std::pow(widthBase, widthParam) * widthMult;
        pulseWidthSamples = int ((pulseWidthMs / 1000.0f) * args.sampleRate);

        float decayParam = params[DECAY_PARAM].getValue() + inputs[DECAY_IN].getVoltage() / 10.0f;
        float decayTimeMs = std::pow(decayBase, decayParam) * decayMult;
        float r162 = (decayTimeMs / 1000.0f) / capVal;
        float r163 = r162 * 200.0f;
        pulseShaper->setResistors (r162, r163);

        float doubleParam = params[DOUBLE_PARAM].getValue() + inputs[DOUBLE_IN].getVoltage() / 10.0f;
        doubleTapGain = -2.0f * doubleParam;
    }

    inline float getPulse() {
        float triggered = trigger.process(inputs[TRIG_IN].getVoltage());
        sampleCount = triggered ? pulseWidthSamples : sampleCount;

        float pulse = sampleCount ? 1.0f : 0.0f;
        sampleCount = sampleCount > 0 ? sampleCount - 1 : 0;
        
        return pulse;
    }

    void process(const ProcessArgs& args) override {
        if(paramDivider.process())
            cookParams(args);

        float pulse = getPulse();
        float env = pulseShaper->processSample(pulse);
        env = env > 0.0f ? env : env * doubleTapGain;

        outputs[ENV_OUT].setVoltage(env * 10.0f);
    }

private:
    enum {
        ParamDivide = 16,
    };

    std::unique_ptr<PulseShaper> pulseShaper;

    dsp::ClockDivider paramDivider;
    dsp::SchmittTrigger trigger;
    int pulseWidthSamples = 0;
    int sampleCount = 0;
    
    float doubleTapGain = 0.0f;
};

struct ChowPulseWidget : ModuleWidget {
    ChowPulseWidget(ChowPulse* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChowPulse.svg")));
        createScrews(*this);

        addInput(createInputCentered<ChowPort>(mm2px(Vec(10.85, 23.75)), module, ChowPulse::WIDTH_IN));
        addInput(createInputCentered<ChowPort>(mm2px(Vec(10.85, 42.75)), module, ChowPulse::DECAY_IN));
        addInput(createInputCentered<ChowPort>(mm2px(Vec(10.85, 61.75)), module, ChowPulse::DOUBLE_IN));

        addParam(createParamCentered<ChowKnob>(mm2px(Vec(29.9, 26.75)), module, ChowPulse::WIDTH_PARAM));
		addParam(createParamCentered<ChowKnob>(mm2px(Vec(29.9, 45.75)), module, ChowPulse::DECAY_PARAM));
		addParam(createParamCentered<ChowKnob>(mm2px(Vec(29.9, 64.75)), module, ChowPulse::DOUBLE_PARAM));

        addInput(createInputCentered<ChowPort>(mm2px(Vec(20.5, 97.5)), module, ChowPulse::TRIG_IN));
		addOutput(createOutputCentered<ChowPort>(mm2px(Vec(20.5, 115.0)), module, ChowPulse::ENV_OUT));
    }
};

Model* modelChowPulse = createModel<ChowPulse, ChowPulseWidget>("ChowPulse");
