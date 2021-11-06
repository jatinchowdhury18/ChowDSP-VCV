#include "../plugin.hpp"
#include "../shared/iir.hpp"
#include "ldr.hpp"

struct ChowPhaserFeedback : Module {
	enum ParamIds {
		SKEW_PARAM,
		FB_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_INPUT,
        LFO_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	ChowPhaserFeedback() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configInput(IN_INPUT, "Audio");
        configInput(LFO_INPUT, "LFO");
        configOutput(OUT_OUTPUT, "Audio");
        configBypass(IN_INPUT, OUT_OUTPUT);

		configParam(SKEW_PARAM, -1.f, 1.f, 0.f, "");
		configParam(FB_PARAM, 0.f, 0.95f, 0.f, "");
	}

    void onReset() override {
        Module::onReset();
        fbFilter.reset();
    }

	void process(const ProcessArgs& args) override {
        // handle LFO
        const auto lfo = inputs[LFO_INPUT].getVoltage() / 5.0f;
        const auto rVal = LDR::getLDRResistance(lfo, params[SKEW_PARAM].getValue());

        // feedback process
        calcCoefs(rVal, -1.0f * params[FB_PARAM].getValue(), args.sampleRate);
        auto y = fbFilter.process(inputs[IN_INPUT].getVoltage());
        outputs[OUT_OUTPUT].setVoltage(std::tanh(y / 5.0f) * 5.0f);
	}

private:
    inline void calcCoefs(float R, float fbAmt, float fs) noexcept
    {
        constexpr float C = (float) 15e-9;
        const float RC = R*C;

        // analog coefs
        const float b0s = RC * RC;
        const float b1s = -2.0f * RC;
        const float b2s = 1.0f;
        const float a0s = b0s * (1.0f + fbAmt);
        const float a1s = -b1s * (1.0f - fbAmt);
        const float a2s = 1.0f + fbAmt;

        // frequency warping
        const float wc = calcPoleFreq (a0s, a1s, a2s);
        const auto K = wc == 0.0f ? 2.0f * fs : wc / std::tan (wc / (2.0f * fs));
        const auto KSq = K * K;

        // bilinear transform
        const float a0 = a0s * KSq + a1s * K + a2s;
        a[1] = 2.0f * (a2s - a0s * KSq) / a0;
        a[2] = (a0s * KSq - a1s * K + a2s) / a0;
        b[0] = (b0s * KSq + b1s * K + b2s) / a0;
        b[1] = 2.0f * (b2s - b0s * KSq) / a0;
        b[2] = (b0s * KSq - b1s * K + b2s) / a0;

        fbFilter.setCoefficients (b, a);
    }

    inline float calcPoleFreq(float a, float b, float c) const noexcept
    {
        auto radicand = b*b - 4.0f*a*c;

        return radicand >= 0.0f ? 0.0f : std::sqrt (-radicand) / (2.0f * a);
    }

    BiquadFilter fbFilter;
    float a[3];
    float b[3];
};


struct ChowPhaserFeedbackWidget : ModuleWidget {
	ChowPhaserFeedbackWidget(ChowPhaserFeedback* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChowPhaseFB.svg")));
        createScrews(*this);

		addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 26.75)), module, ChowPhaserFeedback::SKEW_PARAM));
		addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 51.0)), module, ChowPhaserFeedback::FB_PARAM));

		addInput(createInputCentered<ChowPort>(mm2px(Vec(9.13, 97.5)), module, ChowPhaserFeedback::LFO_INPUT));
		addInput(createInputCentered<ChowPort>(mm2px(Vec(21.3, 97.5)), module, ChowPhaserFeedback::IN_INPUT));
		addOutput(createOutputCentered<ChowPort>(mm2px(Vec(15.25, 115.0)), module, ChowPhaserFeedback::OUT_OUTPUT));
	}
};


Model* modelChowPhaserFeedback = createModel<ChowPhaserFeedback, ChowPhaserFeedbackWidget>("ChowPhaserFeedback");
