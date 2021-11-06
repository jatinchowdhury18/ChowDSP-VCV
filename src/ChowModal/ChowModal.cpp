#include "../plugin.hpp"
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

        configInput(AUDIO_IN, "Audio");
        configInput(FREQ_IN, "Frequency mod.");
        configInput(DECAY_IN, "Decay mod.");
        configInput(AMP_IN, "Amplitude mod.");
        configInput(PHASE_IN, "Phase mod.");
        configOutput(AUDIO_OUT, "Audio");
        configBypass(AUDIO_IN, AUDIO_OUT);

        configParam(FREQ_PARAM, -36.0f, 36.0f, 0.0f, "Frequency", " Hz", dsp::FREQ_SEMITONE, dsp::FREQ_C4);
        configParam(DECAY_PARAM, 0.0f, 1.0f, 0.2f, "Decay", " seconds", 0.0f, decayMult);
        configParam(AMP_PARAM, 0.0f, 1.0f, 0.25f, "Amplitude");
        configParam(PHASE_PARAM, 0.0f, 1.0f, 0.0f, "Phase");

        onSampleRateChange();
	}

    void onSampleRateChange() override {
        mode.prepare((double) getSampleRate());
    }

    void onReset() override {
        Module::onReset();
        mode.reset();
    }

	void process(const ProcessArgs& args) override {
        // set mode frequency
        float freqParam = params[FREQ_PARAM].getValue();
        float freq = std::pow(dsp::FREQ_SEMITONE, freqParam) * dsp::FREQ_C4;
        freq *= std::pow(2, inputs[FREQ_IN].getVoltage());
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
        createScrews(*this);

        addInput(createInputCentered<ChowPort>(mm2px(Vec(10.85, 23.75)), module, ChowModal::FREQ_IN));
        addInput(createInputCentered<ChowPort>(mm2px(Vec(10.85, 42.75)), module, ChowModal::DECAY_IN));
        addInput(createInputCentered<ChowPort>(mm2px(Vec(10.85, 61.75)), module, ChowModal::AMP_IN));
        addInput(createInputCentered<ChowPort>(mm2px(Vec(10.85, 81.0)), module, ChowModal::PHASE_IN));

        addParam(createParamCentered<ChowKnob>(mm2px(Vec(29.9, 26.75)), module, ChowModal::FREQ_PARAM));
		addParam(createParamCentered<ChowKnob>(mm2px(Vec(29.9, 45.75)), module, ChowModal::DECAY_PARAM));
		addParam(createParamCentered<ChowKnob>(mm2px(Vec(29.9, 64.75)), module, ChowModal::AMP_PARAM));
		addParam(createParamCentered<ChowKnob>(mm2px(Vec(29.9, 84.0)), module, ChowModal::PHASE_PARAM));

        addInput(createInputCentered<ChowPort>(mm2px(Vec(20.5, 97.5)), module, ChowModal::AUDIO_IN));
		addOutput(createOutputCentered<ChowPort>(mm2px(Vec(20.5, 115.0)), module, ChowModal::AUDIO_OUT));
	}

    void appendContextMenu(Menu *menu) override {
        menu->addChild(new MenuSeparator());
        addPubToMenu(menu, "https://ccrma.stanford.edu/~jos/smac03maxjos/");
    }
};


Model* modelChowModal = createModel<ChowModal, ChowModalWidget>("ChowModal");