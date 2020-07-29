#include "plugin.hpp"
#include "shared/delay.hpp"
#include "shared/fdn.hpp"

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
        configParam(PRE_DELAY_PARAM, 0.0f, 1.0f, 0.5f, "Time", " ms", maxPreDelayMs, 1.0f);
        configParam(SIZE_PARAM, 0.1f, 1.0f, 0.5f, "Size");
        configParam(T60_LOW_PARAM, 0.5f, 10.0f, 1.0f, "T60 Low", " s");
        configParam(T60_HIGH_PARAM, 0.5f, 10.0f, 0.5f, "T60 High", " s");
        configParam(NUM_DELAYS_PARAM, 1.0f, 16.0f, 4.0f, "# delays");
        configParam(DRYWET_PARAM, 0.0f, 1.0f, 1.0f, "Dry/Wet");
    }

    void process(const ProcessArgs& args) override {
        const float x = inputs[AUDIO_INPUT].getVoltage();

        // process pre-delay
        preDelay.setProcessArgs(args);
        float delayParam = clamp(params[PRE_DELAY_PARAM].getValue(), 0.0f, 1.0f);
        preDelay.setDelayTimeMs(std::pow(maxPreDelayMs, delayParam));
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
    Delay preDelay;
    FDN<16> fdn;
};

struct ChowFDNWidget : ModuleWidget {
    ChowFDNWidget(ChowFDN* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChowFDN.svg")));

        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.0, 10.0)), module, ChowFDN::PRE_DELAY_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.0, 30.0)), module, ChowFDN::SIZE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.0, 50.0)), module, ChowFDN::T60_HIGH_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.0, 70.0)), module, ChowFDN::T60_LOW_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(5.00, 90.0)), module, ChowFDN::NUM_DELAYS_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.0, 90.0)), module, ChowFDN::DRYWET_PARAM));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.507, 105.0)), module, ChowFDN::AUDIO_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 120.0)), module, ChowFDN::AUDIO_OUTPUT));
    }

};

Model* modelChowFDN = createModel<ChowFDN, ChowFDNWidget>("ChowFDN");
