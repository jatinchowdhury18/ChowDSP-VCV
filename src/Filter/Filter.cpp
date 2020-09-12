#include "../plugin.hpp"
#include "../shared/oversampling.hpp"
#include "../shared/nl_biquad.hpp"
#include "NewtonRaphson.hpp"

using namespace std::placeholders;

namespace {
    constexpr float highFreq = 2000.0f;
    constexpr float lowFreq = 5.0f;
    constexpr float qMult = 0.25f;
    constexpr float qBase = 19.75f;
    constexpr float qOff = 0.05f;
}

struct Filter : Module {
	enum ParamIds {
        FREQ_PARAM,
        Q_PARAM,
        GAIN_PARAM,
        DRIVE_PARAM,
        FB_DRIVE_PARAM,
        FB_PARAM,
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

	Filter() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(FREQ_PARAM, 0.0f, 1.0f, 0.5f, "Freq.", " Hz", highFreq / lowFreq, lowFreq);
        configParam(Q_PARAM, 0.0f, 1.0f, 0.324f, "Q", "", qBase, qMult, qOff);
        configParam(GAIN_PARAM, -30.0f, 30.0f, -6.0f, "Gain", " dB");
        configParam(DRIVE_PARAM, 1.0f, 10.0f, 1.0f, "Drive");
        configParam(FB_DRIVE_PARAM, 1.0f, 10.0f, 1.0f, "FB Drive");
        configParam(FB_PARAM, 0.0f, 0.95f, 0.0f, "Feedback");

        oversample.osProcess = std::bind(&Filter::processOS, this, _1);
        onSampleRateChange();

        nrSolver.f_NL = [=] (float x) -> float {
            return std::tanh(x) / params[FB_DRIVE_PARAM].getValue();
        };

        nrSolver.f_NL_prime = [=] (float x) -> float {
            const auto coshX = std::cosh(x);
            return 1.0f / (coshX * coshX);
        };
	}

    void onSampleRateChange() override {
        float newSampleRate = getSampleRate();
        oversample.reset(newSampleRate);
    }

	void process(const ProcessArgs& args) override {
        auto freq = pow(highFreq / lowFreq, params[FREQ_PARAM].getValue()) * lowFreq;
        auto q = pow(qBase, params[Q_PARAM].getValue()) * qMult + qOff;
        auto gain = pow(10.0f, params[GAIN_PARAM].getValue() / 20.0f);

        filter.setParameters(BiquadFilter::PEAK, freq / (OSRatio * args.sampleRate), q, gain);
        filter.setDrive(params[DRIVE_PARAM].getValue());

        nrSolver.driveParam = params[FB_DRIVE_PARAM].getValue();
        nrSolver.fbParam = params[FB_PARAM].getValue();

        float x = inputs[AUDIO_IN].getVoltage();
        float y = oversample.process(x);
        outputs[AUDIO_OUT].setVoltage(y);
	}

    // oversampled process
    inline float processOS(float x) {
        return nrSolver.process(x, filter.b[0], filter.z[1], [=] (float x) { filter.process(x); });
    }

private:
enum {
        OSRatio = 2,
    };

    OversampledProcess<OSRatio> oversample;
    NLBiquad filter;
    // BiquadFilter filter;
    DFLFilter<4> nrSolver;
};


struct FilterWidget : ModuleWidget {
	FilterWidget(Filter* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Filter.svg")));

        createScrews (*this);

        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25,  5.0)), module, Filter::FREQ_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 20.0)), module, Filter::Q_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 35.0)), module, Filter::GAIN_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 50.0)), module, Filter::DRIVE_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 65.0)), module, Filter::FB_DRIVE_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 80.0)), module, Filter::FB_PARAM));

        addInput(createInputCentered<ChowPort>(mm2px(Vec(15.25, 99.0)), module, Filter::AUDIO_IN));
        addOutput(createOutputCentered<ChowPort>(mm2px(Vec(15.25, 115.0)), module, Filter::AUDIO_OUT));
	}
};


Model* modelFilter = createModel<Filter, FilterWidget>("Filter");