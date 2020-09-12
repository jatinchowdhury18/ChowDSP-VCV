#include "../plugin.hpp"
#include "GenSVF.hpp"

namespace {
    constexpr float highFreq = 20000.0f;
    constexpr float lowFreq = 20.0f;
}

struct Werner : Module {
    enum ParamIds {
        FREQ_PARAM,
        FB_PARAM,
        DAMPING_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
        AUDIO_IN,
		NUM_INPUTS
	};
	enum OutputIds {
        AUDIO_OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

    Werner() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(FREQ_PARAM, 0.0f, 1.0f, 0.5f, "Freq.", " Hz", highFreq / lowFreq, lowFreq);
        configParam(DAMPING_PARAM, 0.25f, 1.25f, 0.5f, "Damp");
        configParam(FB_PARAM, 0.0f, 0.95f, 0.5f, "Feedback");

        svf.reset();
    }

    void onReset() override {
        Module::onReset();
        svf.reset();
    }

    void process(const ProcessArgs& args) override {
        float r = params[DAMPING_PARAM].getValue();
        float k = params[FB_PARAM].getValue();
        auto freq = pow(highFreq / lowFreq, params[FREQ_PARAM].getValue()) * lowFreq;
        float wc = (freq / args.sampleRate) * M_PI_2;
        svf.calcCoefs(r, k, wc);

        float x = inputs[AUDIO_IN].getVoltage();
        float y = svf.process(x);
        outputs[AUDIO_OUT].setVoltage(y);
	}

private:
    GeneralSVF svf;
};

struct WernerWidget : ModuleWidget {
	WernerWidget(Werner* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Werner.svg")));

        createScrews (*this);

        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 20.0)), module, Werner::FREQ_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 35.0)), module, Werner::FB_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 50.0)), module, Werner::DAMPING_PARAM));

        addInput(createInputCentered<ChowPort>(mm2px(Vec(15.25, 99.0)), module, Werner::AUDIO_IN));
        addOutput(createOutputCentered<ChowPort>(mm2px(Vec(15.25, 115.0)), module, Werner::AUDIO_OUT));
	}
};

Model* modelWerner = createModel<Werner, WernerWidget>("Werner");
