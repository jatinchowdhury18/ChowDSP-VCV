#include "plugin.hpp"


struct ChowPhaserMod : Module {
	enum ParamIds {
		SKEW_PARAM,
		MOD_PARAM,
        STAGES_PARAM,
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

	ChowPhaserMod() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(SKEW_PARAM, -1.f, 1.f, 0.f, "");
		configParam(MOD_PARAM, 0.f, 1.f, 0.f, "");
        configParam(STAGES_PARAM, 1.0f, 50.0f, 8.0f, "");

        std::fill(z, &z[maxNumStages], 0.0f);
	}

	void process(const ProcessArgs& args) override {
        // handle LFO
        constexpr float maxDepth = 20.0f;
        const auto skewVal = std::pow(2.0f, params[SKEW_PARAM].getValue());
        const auto lfoVal = lightShape(inputs[LFO_INPUT].getVoltage() / 5.0f, skewVal);
        const auto lightVal = (maxDepth + 0.1f) - (lfoVal * maxDepth);
        const auto rVal = 100000.0f * std::pow(lightVal / 0.1f, -0.75f);

        // process mod
        const auto mod = params[MOD_PARAM].getValue();
        const auto numStages = params[STAGES_PARAM].getValue();

        calcCoefs(rVal, args.sampleRate);
        const auto x = inputs[IN_INPUT].getVoltage();
        float y = x;
        for(int stage = 0; stage < (int) numStages; ++stage)
            y = processStage(y, stage);

        // fractional stage
        float stageFrac = numStages - (int) numStages;
        y = stageFrac * processStage(y, (int) numStages) + (1.0f - stageFrac) * y;

        // mix
        y = mod * y + (1.0f - mod) * x;
        outputs[OUT_OUTPUT].setVoltage(y);
	}

private:
    inline float lightShape(float x, float skewPow) const noexcept
    {
        x = clamp(x, -1.0f, 1.0f);
        return (std::pow((x + 1.0f) / 2.0f, skewPow) * 2.0f) - 1.0f;
    }

    inline float processStage(float x, int stage)
    {
        float y = z[stage] + x * b[0];
        z[stage] = x * b[1] - y * a[1];
        return y;
    }

    inline void calcCoefs(float R, float fs)
    {
        // component values
        constexpr float C = (float) 25e-9;
        const float RC = R*C;

        // analog coefs
        const float b0s = RC;
        const float b1s = -1.0f;
        const float a0s = b0s;
        const float a1s = 1.0f;

        // bilinear transform
        const auto K = 2.0f * fs;
        const auto a0 = a0s * K + a1s;
        b[0] = ( b0s * K + b1s) / a0;
        b[1] = (-b0s * K + b1s) / a0;
        a[0] = 1.0f;
        a[1] = (-a0s * K + a1s) / a0;
    }

    enum {
        maxNumStages = 52,
    };

    float a[2] = {1.0f, 0.0f};
    float b[2] = {1.0f, 0.0f};
    float z[maxNumStages];
};


struct ChowPhaserModWidget : ModuleWidget {
	ChowPhaserModWidget(ChowPhaserMod* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChowPhaseMod.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.764, 20.25)), module, ChowPhaserMod::SKEW_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.502, 36.559)), module, ChowPhaserMod::MOD_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.502, 53.932)), module, ChowPhaserMod::STAGES_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.507, 73.872)), module, ChowPhaserMod::LFO_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.507, 91.109)), module, ChowPhaserMod::IN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 111.385)), module, ChowPhaserMod::OUT_OUTPUT));
	}
};


Model* modelChowPhaserMod = createModel<ChowPhaserMod, ChowPhaserModWidget>("ChowPhaserMod");