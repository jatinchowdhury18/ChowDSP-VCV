#include "../plugin.hpp"
#include "BaxandallEQ.hpp"
#include "ClippingStage.hpp"
#include "../shared/VariableOversampling.hpp"
#include "../shared/shelf_filter.hpp"

struct ChowDer : Module {
	enum ParamIds {
        BASS_PARAM,
        TREBLE_PARAM,
        DRIVE_PARAM,
        BIAS_PARAM,
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

	ChowDer() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configInput(AUDIO_IN, "Audio");
        configOutput(AUDIO_OUT, "Audio");
        configBypass(AUDIO_IN, AUDIO_OUT);

        configParam(BASS_PARAM, -1.0f, 1.0f, 0.0f, "Bass");
        configParam(TREBLE_PARAM, -1.0f, 1.0f, 0.0f, "Treble");
        configParam(DRIVE_PARAM, 0.0f, 1.0f, 0.5f, "Drive");
        configParam(BIAS_PARAM, 0.0f, 1.0f, 0.0f, "Bias");

        oversample.setOversamplingIndex(1); // default 2x oversampling
        onSampleRateChange();
        paramDivider.setDivision(ParamDivide);
	}

    void onSampleRateChange() override {
        float newSampleRate = getSampleRate();

        oversample.reset(newSampleRate);
        clipper.reset(new ClippingStage(newSampleRate * oversample.getOversamplingRatio()));
        dcBlocker.setParameters(BiquadFilter::HIGHPASS, 30.0f / newSampleRate, M_SQRT1_2, 1.0f);
        cookParams(newSampleRate);
    }

    void onReset() override {
        Module::onReset();
        onSampleRateChange();
    }

    void cookParams(float fs) {
        auto lowGain = dsp::dbToAmplitude(params[BASS_PARAM].getValue() * 9.0f - 20.0f);
        auto highGain = dsp::dbToAmplitude(params[TREBLE_PARAM].getValue() * 9.0f - 20.0f);
        shelfFilter.calcCoefs(lowGain, highGain, 600.0f, fs);

        driveGain = dsp::dbToAmplitude(params[DRIVE_PARAM].getValue() * 30.0f);
        bias = params[BIAS_PARAM].getValue() * 2.5f;
    }

	void process(const ProcessArgs& args) override {
        if(paramDivider.process())
            cookParams(args.sampleRate);

        float x = inputs[AUDIO_IN].getVoltage();
        x = driveGain * shelfFilter.process(x) + bias;

        oversample.upsample(x);
        float* osBuffer = oversample.getOSBuffer();
        for(int k = 0; k < oversample.getOversamplingRatio(); k++)
            osBuffer[k] = clipper->processSample(osBuffer[k]);
        float y = oversample.downsample();

        outputs[AUDIO_OUT].setVoltage(dcBlocker.process(y));
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
    enum {
        ParamDivide = 64,
    };

    float driveGain = 1.0f;
    float bias = 0.0f;
    dsp::ClockDivider paramDivider;

    BiquadFilter dcBlocker;
    ShelfFilter shelfFilter;
    std::unique_ptr<ClippingStage> clipper;
};


struct ChowDerWidget : ModuleWidget {
	ChowDerWidget(ChowDer* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChowDer.svg")));

        createScrews (*this);

        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 23.0)), module, ChowDer::BASS_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 43.0)), module, ChowDer::TREBLE_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 63.0)), module, ChowDer::DRIVE_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 83.0)), module, ChowDer::BIAS_PARAM));

        addInput(createInputCentered<ChowPort>(mm2px(Vec(15.25, 99.0)), module, ChowDer::AUDIO_IN));
        addOutput(createOutputCentered<ChowPort>(mm2px(Vec(15.25, 115.0)), module, ChowDer::AUDIO_OUT));
	}

    void appendContextMenu(Menu *menu) override {
        menu->addChild(new MenuSeparator());
        dynamic_cast<ChowDer*> (module)->oversample.addContextMenu(menu, module);
    }
};


Model* modelChowDer = createModel<ChowDer, ChowDerWidget>("ChowDer");