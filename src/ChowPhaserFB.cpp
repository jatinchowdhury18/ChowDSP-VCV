#include "plugin.hpp"


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
		configParam(SKEW_PARAM, -1.f, 1.f, 0.f, "");
		configParam(FB_PARAM, 0.f, 0.95f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
        // handle LFO
        constexpr float maxDepth = 20.0f;
        const auto skewVal = std::pow(2.0f, params[SKEW_PARAM].getValue());
        const auto lfoVal = lightShape(inputs[LFO_INPUT].getVoltage() / 5.0f, skewVal);
        const auto lightVal = (maxDepth + 0.1f) - (lfoVal * maxDepth);
        const auto rVal = 100000.0f * std::pow(lightVal / 0.1f, -0.75f);

        // feedback process
        calcCoefs(rVal, -1.0f * params[FB_PARAM].getValue(), args.sampleRate);
        auto y = fbFilter.process(inputs[IN_INPUT].getVoltage());
        outputs[OUT_OUTPUT].setVoltage(y);
	}

private:
    inline float lightShape(float x, float skewPow) const noexcept
    {
        x = clamp(x, -1.0f, 1.0f);
        return (std::pow((x + 1.0f) / 2.0f, skewPow) * 2.0f) - 1.0f;
    }

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
        a[0] = 2.0f * (a2s - a0s * KSq) / a0;
        a[1] = (a0s * KSq - a1s * K + a2s) / a0;
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

    dsp::IIRFilter<3, 3, float> fbFilter;
    float a[2];
    float b[3];
};


struct ChowPhaserFeedbackWidget : ModuleWidget {
	ChowPhaserFeedbackWidget(ChowPhaserFeedback* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChowPhaseFB.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.234, 19.721)), module, ChowPhaserFeedback::SKEW_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.502, 41.37)), module, ChowPhaserFeedback::FB_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.507, 69.863)), module, ChowPhaserFeedback::LFO_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.507, 91.109)), module, ChowPhaserFeedback::IN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 111.385)), module, ChowPhaserFeedback::OUT_OUTPUT));
	}
};


Model* modelChowPhaserFeedback = createModel<ChowPhaserFeedback, ChowPhaserFeedbackWidget>("ChowPhaserFeedback");
