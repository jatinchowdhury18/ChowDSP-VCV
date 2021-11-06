#include "../plugin.hpp"
#include "../shared/delay_line.hpp"
#include "fdn.hpp"

struct ChowFDN : Module {
    enum ParamIds {
        PRE_DELAY_PARAM,
        SIZE_PARAM,
        T60_HIGH_PARAM,
        T60_LOW_PARAM,
        NUM_DELAYS_PARAM,
        DRYWET_PARAM,
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

    ChowFDN() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configInput(AUDIO_INPUT, "Audio");
        configOutput(AUDIO_OUTPUT, "Audio");
        configBypass(AUDIO_INPUT, AUDIO_OUTPUT);

        configParam(PRE_DELAY_PARAM, 0.0f, 1.0f, 0.5f, "Time", " ms", maxPreDelayMs, 1.0f);
        configParam(SIZE_PARAM, 0.1f, 1.0f, 0.5f, "Size");
        configParam(T60_LOW_PARAM, 0.5f, 10.0f, 1.0f, "T60 Low", " s");
        configParam(T60_HIGH_PARAM, 0.5f, 10.0f, 0.5f, "T60 High", " s");
        configParam(NUM_DELAYS_PARAM, 1.0f, 16.0f, 4.0f, "# delays");
        configParam(DRYWET_PARAM, 0.0f, 1.0f, 1.0f, "Dry/Wet");

        preDelay.reset();
    }

    void onReset() override {
        Module::onReset();
        
        preDelay.reset();
        fdn.reset();
    }

    void process(const ProcessArgs& args) override {
        const float x = inputs[AUDIO_INPUT].getVoltage();

        // process pre-delay
        float delayMs = std::pow(maxPreDelayMs, params[PRE_DELAY_PARAM].getValue());
        preDelay.setDelay(args.sampleRate * delayMs / 1000.0f);
        float y = preDelay.process(x);

        // process FDN
        float size = params[SIZE_PARAM].getValue();
        float t60low = params[T60_LOW_PARAM].getValue();
        float t60high = params[T60_HIGH_PARAM].getValue();
        int numDelays = (int) params[NUM_DELAYS_PARAM].getValue();
        fdn.prepare(args, size, t60low, t60high, numDelays);
        y = fdn.process(y, numDelays);

        // process dry/wet
        float mix = params[DRYWET_PARAM].getValue();
        outputs[AUDIO_OUTPUT].setVoltage(mix * y + (1.0f - mix) * x);
    }

private:
    const float maxPreDelayMs = 500.0f;
    DelayLine<float, DelayLineInterpolationTypes::Lagrange3rd> preDelay;
    FDN<16> fdn;
};

struct ChowFDNWidget : ModuleWidget {
    ChowFDNWidget(ChowFDN* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChowFDN.svg")));
        createScrews(*this);

        constexpr double x1 = 10.85;
        constexpr double x2 = 29.9;

        addParam(createParamCentered<ChowKnob>(mm2px(Vec(x1, 26.75)), module, ChowFDN::PRE_DELAY_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(x2, 26.75)), module, ChowFDN::SIZE_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(x1, 51.0)), module, ChowFDN::T60_LOW_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(x2, 51.0)), module, ChowFDN::T60_HIGH_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(x1, 75.5)), module, ChowFDN::NUM_DELAYS_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(x2, 75.5)), module, ChowFDN::DRYWET_PARAM));

        addInput(createInputCentered<ChowPort>(mm2px(Vec(20.5, 97.5)), module, ChowFDN::AUDIO_INPUT));
		addOutput(createOutputCentered<ChowPort>(mm2px(Vec(20.5, 115.0)), module, ChowFDN::AUDIO_OUTPUT));
    }

};

Model* modelChowFDN = createModel<ChowFDN, ChowFDNWidget>("ChowFDN");
