#include "plugin.hpp"
#include "HysteresisProcessing.h"

struct ChowTape : Module {
	enum ParamIds {
		BIAS_PARAM,
		SAT_PARAM,
		DRIVE_PARAM,
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

	ChowTape() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(BIAS_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SAT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(DRIVE_PARAM, 0.f, 1.f, 0.f, "");

        hysteresis.reset();
        hysteresis.setSolver (SolverType::RK2);
	}
    
    float calcMakeup(float width, float sat) const noexcept
    {
        return (1.0f + 0.6f * width) / (0.5f + 1.5f * (1.0f - sat));
    }

	void process(const ProcessArgs& args) override {
        hysteresis.setSampleRate (args.sampleRate);

        float width = 1.0f - params[BIAS_PARAM].getValue();
        float sat = params[SAT_PARAM].getValue();
        float drive = params[DRIVE_PARAM].getValue();

        hysteresis.cook (drive, width, sat, false);

        float x = inputs[AUDIO_INPUT].getVoltage() / 5.0f;
        float y = hysteresis.process (x) * calcMakeup (width, sat);

        outputs[AUDIO_OUTPUT].setVoltage(y * 5.0f);
	}

private:
    HysteresisProcessing hysteresis;
};


struct ChowTapeWidget : ModuleWidget {
	ChowTapeWidget(ChowTape* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChowTape.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.234, 19.721)), module, ChowTape::BIAS_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.502, 41.37)), module, ChowTape::SAT_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 64.25)), module, ChowTape::DRIVE_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.507, 91.109)), module, ChowTape::AUDIO_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 111.385)), module, ChowTape::AUDIO_OUTPUT));
	}
};


Model* modelChowTape = createModel<ChowTape, ChowTapeWidget>("ChowTape");