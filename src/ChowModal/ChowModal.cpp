#include "plugin.hpp"
#include "mode.hpp"

struct ChowModal : Module {
	enum ParamIds {
        FREQ_PARAM,
        DECAY_PARAM,
        AMP_PARAM,
        PHASE_PARAM,
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

	ChowModal() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(FREQ_PARAM, -54.0f, 54.0f, 0.0f, "Frequency", " Hz", dsp::FREQ_SEMITONE, dsp::FREQ_C4);
        configParam(DECAY_PARAM, 0.0f, 1.0f, 0.5f, "Decay", " seconds", 0.0f, decayMult);
        configParam(AMP_PARAM, 0.0f, 1.0f, 1.0f, "Amplitude");
        configParam(PHASE_PARAM, 0.0f, 1.0f, 0.5f, "Phase");

        mode.prepare((double) APP->engine->getSampleRate());
	}

    void onSampleRateChange() override {
        mode.prepare((double) APP->engine->getSampleRate());
    }

	void process(const ProcessArgs& args) override {
        // set mode frequency
        float freqParam = params[FREQ_PARAM].getValue();
        float freq = std::pow(dsp::FREQ_SEMITONE, freqParam) * dsp::FREQ_C4;
        mode.setFreq(freq);
        
        // set mode decay
        float decayParam = params[DECAY_PARAM].getValue();
        mode.setDecay(decayMult * decayParam);

        // set complex amplitude
        float amp = params[AMP_PARAM].getValue();
        float phase = params[PHASE_PARAM].getValue();
        mode.setAmp(0.1f * amp, phase);

        mode.updateParams();

        // process audio
        float x = inputs[AUDIO_IN].getVoltage();
        float y = mode.getNextSample(x);
        outputs[AUDIO_OUT].setVoltage(y);
	}

private:
    const float decayMult = 5.0f;
    Mode mode;
};


struct ChowModalWidget : ModuleWidget {
	ChowModalWidget(ChowModal* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChowModal.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.25, 20.0)), module, ChowModal::FREQ_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.25, 37.5)), module, ChowModal::DECAY_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.25, 55.0)), module, ChowModal::AMP_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.25, 72.5)), module, ChowModal::PHASE_PARAM));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.25, 93.0)), module, ChowModal::AUDIO_IN));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.25, 113.0)), module, ChowModal::AUDIO_OUT));
	}
};


Model* modelChowModal = createModel<ChowModal, ChowModalWidget>("ChowModal");