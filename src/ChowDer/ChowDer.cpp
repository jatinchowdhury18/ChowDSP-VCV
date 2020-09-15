#include "../plugin.hpp"
#include "BaxandallEQ.hpp"
#include "ClippingStage.hpp"
#include "../shared/oversampling.hpp"
#include "../shared/shelf_filter.hpp"

using namespace std::placeholders;

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

        configParam(BASS_PARAM, 0.0f, 1.0f, 0.5f, "Bass");
        configParam(TREBLE_PARAM, 0.0f, 1.0f, 0.5f, "Treble");
        configParam(DRIVE_PARAM, 0.0f, 1.0f, 0.5f, "Drive");
        configParam(BIAS_PARAM, 0.0f, 1.0f, 0.0f, "Bias");

        onSampleRateChange();
        paramDivider.setDivision(ParamDivide);
	}

    void onSampleRateChange() override {
        float newSampleRate = getSampleRate();
        oversample.reset(newSampleRate);
        clipper.reset((double) newSampleRate * OSRatio);
        dcBlocker.setParameters(BiquadFilter::HIGHPASS, 30.0f / newSampleRate, M_SQRT1_2, 1.0f);
        cookParams(newSampleRate);
    }

    void cookParams(float fs) {
        auto lowGain = dsp::dbToAmplitude(params[BASS_PARAM].getValue() * 12.0f - 6.0f);
            auto highGain = dsp::dbToAmplitude(params[TREBLE_PARAM].getValue() * 12.0f - 6.0f);
            shelfFilter.calcCoefs(lowGain, highGain, 500.0f, fs);

            driveGain = dsp::dbToAmplitude(params[DRIVE_PARAM].getValue() * 30.0f);
            bias = params[BIAS_PARAM].getValue() * 2.5f;
    }

	void process(const ProcessArgs& args) override {
        if(paramDivider.process())
            cookParams(args.sampleRate);

        float x = inputs[AUDIO_IN].getVoltage();
        x = driveGain * shelfFilter.process(x) + bias;

        oversample.upsample(x);
        for(int k = 0; k < OSRatio; k++)
            oversample.osBuffer[k] = clipper.processSample(oversample.osBuffer[k]);
        float y = oversample.downsample();

        outputs[AUDIO_OUT].setVoltage(dcBlocker.process(y));
	}

private:
    enum {
        OSRatio = 2,
        ParamDivide = 64,
    };

    float driveGain = 1.0f;
    float bias = 0.0f;
    dsp::ClockDivider paramDivider;

    BiquadFilter dcBlocker;
    OversampledProcess<OSRatio> oversample;
    ShelfFilter shelfFilter;
    ClippingStage clipper;
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
};


Model* modelChowDer = createModel<ChowDer, ChowDerWidget>("ChowDer");