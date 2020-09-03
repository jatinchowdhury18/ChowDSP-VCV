#include "../plugin.hpp"
#include "BaxandallEQ.hpp"
#include "ClippingStage.hpp"
#include "../shared/oversampling.hpp"

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

        oversample.osProcess = std::bind(&ChowDer::processOS, this, _1);
        onSampleRateChange();
	}

    void onSampleRateChange() override {
        float newSampleRate = APP->engine->getSampleRate();
        oversample.reset(newSampleRate);
        baxandall.reset((double) newSampleRate * OSRatio);
        clipper.reset((double) newSampleRate * OSRatio);
        dcBlocker.setParameters(BiquadFilter::HIGHPASS, 30.0f / newSampleRate, M_SQRT1_2, 1.0f);
    }

	void process(const ProcessArgs& args) override {
        // handle parameters
        baxandall.setBass(params[BASS_PARAM].getValue());
        baxandall.setTreble(params[TREBLE_PARAM].getValue());

        float driveGain = dsp::dbToAmplitude(params[DRIVE_PARAM].getValue() * 30.0f);
        float bias = params[BIAS_PARAM].getValue() * 2.5f;

        float x = driveGain * (inputs[AUDIO_IN].getVoltage() + bias);
        float y = oversample.process(x);
        outputs[AUDIO_OUT].setVoltage(dcBlocker.process(y));
	}

    // oversampled process
    inline float processOS(float x) {
        float y = baxandall.processSample(x);
        y = clipper.processSample(y);

        return y;
    }

private:
enum {
        OSRatio = 1,
    };

    BiquadFilter dcBlocker;
    OversampledProcess<OSRatio> oversample;
    BaxandallEQ baxandall;
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