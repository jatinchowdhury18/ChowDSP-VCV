#include "plugin.hpp"
#include "Mode.hpp"

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
        FREQ_IN,
        DECAY_IN,
        AMP_IN,
        PHASE_IN,
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

        configParam(FREQ_PARAM, -36.0f, 36.0f, 0.0f, "Frequency", " Hz", dsp::FREQ_SEMITONE, dsp::FREQ_C4);
        configParam(DECAY_PARAM, 0.0f, 1.0f, 0.2f, "Decay", " seconds", 0.0f, decayMult);
        configParam(AMP_PARAM, 0.0f, 1.0f, 0.25f, "Amplitude");
        configParam(PHASE_PARAM, 0.0f, 1.0f, 0.0f, "Phase");

        mode.prepare((double) APP->engine->getSampleRate());
	}

    void onSampleRateChange() override {
        mode.prepare((double) APP->engine->getSampleRate());
    }

	void process(const ProcessArgs& args) override {
        // set mode frequency
        float freqParam = params[FREQ_PARAM].getValue();
        freqParam += 2 * inputs[FREQ_IN].getVoltage();
        float freq = std::pow(dsp::FREQ_SEMITONE, freqParam) * dsp::FREQ_C4;
        mode.setFreq(freq);
        
        // set mode decay
        float decayParam = params[DECAY_PARAM].getValue();
        decayParam += inputs[DECAY_IN].getVoltage() / 10.0f;
        decayParam = clamp(decayParam, 0.0f, 1.5f);
        mode.setDecay(decayMult * decayParam);

        // set complex amplitude
        float amp = params[AMP_PARAM].getValue() + 0.1f * inputs[AMP_IN].getVoltage();
        float phase = params[PHASE_PARAM].getValue() + 0.1f * inputs[PHASE_IN].getVoltage();
        mode.setAmp(0.01f * amp, phase);

        mode.updateParams();

        // process audio
        float x = inputs[AUDIO_IN].getVoltage();
        float y = mode.getNextSample(x);

        dcBlocker.setParameters(dsp::BiquadFilter::HIGHPASS, 30.0f / args.sampleRate, M_SQRT1_2, 1.0f);
        outputs[AUDIO_OUT].setVoltage(dcBlocker.process(y));
	}

private:
    const float decayMult = 7.0f;
    Mode mode;
    dsp::BiquadFilter dcBlocker;
};


struct ChowModalWidget : ModuleWidget {
	ChowModalWidget(ChowModal* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChowModal.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.0, 25.0)), module, ChowModal::FREQ_IN));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.0, 42.5)), module, ChowModal::DECAY_IN));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.0, 60.0)), module, ChowModal::AMP_IN));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.0, 77.5)), module, ChowModal::PHASE_IN));

        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(21.5, 25.0)), module, ChowModal::FREQ_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(21.5, 42.5)), module, ChowModal::DECAY_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(21.5, 60.0)), module, ChowModal::AMP_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(21.5, 77.5)), module, ChowModal::PHASE_PARAM));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.25, 93.0)), module, ChowModal::AUDIO_IN));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.25, 113.0)), module, ChowModal::AUDIO_OUT));
	}
};


Model* modelChowModal = createModel<ChowModal, ChowModalWidget>("ChowModal");