#include "../../plugin.hpp"
#include "../../shared/iir.hpp"
#include "HysteresisProcessing.hpp"
#include "../../shared/VariableOversampling.hpp"

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

        configInput(AUDIO_INPUT, "Audio");
        configOutput(AUDIO_OUTPUT, "Audio");
        configBypass(AUDIO_INPUT, AUDIO_OUTPUT);

		configParam(BIAS_PARAM, 0.f, 1.f, 0.5f, "");
		configParam(SAT_PARAM, 0.f, 1.f, 0.5f, "");
		configParam(DRIVE_PARAM, 0.f, 1.f, 0.5f, "");

        hysteresis.reset();
        hysteresis.setSolver(SolverType::NR4);
        oversample.setOversamplingIndex(2); // default 4x oversampling

        onSampleRateChange();
	}

    void onSampleRateChange() override {
        float newSampleRate = getSampleRate();
        hysteresis.setSampleRate(newSampleRate * oversample.getOversamplingRatio());
        oversample.reset(newSampleRate);
    }

    void onReset() override {
        Module::onReset();
        
        oversample.reset(getSampleRate());
        hysteresis.reset();
    }

	void process(const ProcessArgs& args) override {
        // set hysteresis params
        float width = 1.0f - params[BIAS_PARAM].getValue();
        float sat = params[SAT_PARAM].getValue();
        float drive = params[DRIVE_PARAM].getValue();

        hysteresis.cook (drive, width, sat, false);

        // get input
        float x = clamp(inputs[AUDIO_INPUT].getVoltage() / 5.0f, -1.0f, 1.0f);

        // process hysteresis
        oversample.upsample(x);
        float* osBuffer = oversample.getOSBuffer();
        for(int k = 0; k < oversample.getOversamplingRatio(); k++)
            osBuffer[k] = (float) hysteresis.process((double) osBuffer[k]);
        float y = oversample.downsample();

        // process DC blocker
        dcBlocker.setParameters(BiquadFilter::HIGHPASS, 30.0f / args.sampleRate, M_SQRT1_2, 1.0f);

        outputs[AUDIO_OUTPUT].setVoltage(y * 4.18f);
	}

    json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "osIdx", json_integer(oversample.getOversamplingIndex()));
        return rootJ;
    }

    void dataFromJson(json_t* rootJ) override {
        if(auto* osJson = json_object_get(rootJ, "osIdx"))
            oversample.setOversamplingIndex (json_integer_value(osJson));
    }

    VariableOversampling<> oversample;

private:
    HysteresisProcessing hysteresis;
    BiquadFilter dcBlocker;
};


struct ChowTapeWidget : ModuleWidget {
	ChowTapeWidget(ChowTape* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChowTape.svg")));
		createScrews(*this);

        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 26.75)), module, ChowTape::BIAS_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 51.0)), module, ChowTape::SAT_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 75.5)), module, ChowTape::DRIVE_PARAM));

        addInput(createInputCentered<ChowPort>(mm2px(Vec(15.25, 97.5)), module, ChowTape::AUDIO_INPUT));
        addOutput(createOutputCentered<ChowPort>(mm2px(Vec(15.25, 115.0)), module, ChowTape::AUDIO_OUTPUT));
	}

    void appendContextMenu(Menu *menu) override {
        menu->addChild(new MenuSeparator());
        addPubToMenu(menu, "http://dafx2019.bcu.ac.uk/papers/DAFx2019_paper_3.pdf");
        dynamic_cast<ChowTape*> (module)->oversample.addContextMenu(menu, module);
    }
};


Model* modelChowTape = createModel<ChowTape, ChowTapeWidget>("ChowTape");