#include "../plugin.hpp"
#include "../shared/ChoiceQuantity.hpp"
#include "FullFilter.h"

namespace {
    constexpr float highFreq = 20000.0f;
    constexpr float lowFreq = 20.0f;

    enum {
        ParamDivide = 16,
    };
}

struct TripoleFilter : Module {
    enum ParamIds {
        FREQ_PARAM,
        FREQ_ATTEN_PARAM,
        RESONANCE_PARAM,
        RESONANCE_ATTEN_PARAM,
        MODE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
        AUDIO_IN,
        FREQ_IN,
        RESONANCE_IN,
		NUM_INPUTS
	};
	enum OutputIds {
        STAGE1_OUT,
        STAGE2_OUT,
        STAGE3_OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

    TripoleFilter() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configInput(AUDIO_IN, "Audio");
        configInput(FREQ_IN, "Frequency mod.");
        configInput(RESONANCE_IN, "Resonance mod.");
        configOutput(STAGE1_OUT, "Stage 1");
        configOutput(STAGE2_OUT, "Stage 2");
        configOutput(STAGE3_OUT, "Stage 3");
        configBypass(AUDIO_IN, STAGE1_OUT);
        configBypass(AUDIO_IN, STAGE2_OUT);
        configBypass(AUDIO_IN, STAGE3_OUT);

        configParam(FREQ_PARAM, 0.0f, 1.0f, 0.5f, "Freq.", " Hz", highFreq / lowFreq, lowFreq);
        configParam(FREQ_ATTEN_PARAM, -1.0, 1.f, 0.0f, "Freq atten");
        configParam(RESONANCE_PARAM, 0.0f, 0.95f, 0.5f, "Resonance");
        configParam(RESONANCE_ATTEN_PARAM, -1.0, 1.f, 0.0f, "Resonance atten");

        configParam<ChoiceQuantity>(MODE_PARAM, 0.0f, 4.0f, 0.0f, "Mode");

        // oversample.setOversamplingIndex(1); // default 2x oversampling
        onSampleRateChange();
        paramDivider.setDivision(ParamDivide);
    }

    void onReset() override {
        Module::onReset();

        // oversample.reset(getSampleRate());
        filter.reset(getSampleRate());
    }

    void onSampleRateChange() override {
        filter.reset(getSampleRate());
        // oversample.reset(newSampleRate);
        cookParams();
    }

    void cookParams() {
        float k = params[RESONANCE_PARAM].getValue() + (inputs[RESONANCE_IN].getVoltage() * params[RESONANCE_ATTEN_PARAM].getValue() / 10.0f);
        k = clamp(k, 0.0f, 1.0f);

        float freqParam = params[FREQ_PARAM].getValue() + (inputs[FREQ_IN].getVoltage() * params[FREQ_ATTEN_PARAM].getValue() / 10.0f);
        freqParam = clamp(freqParam, 0.0f, 1.0f);
        auto freq = pow(highFreq / lowFreq, freqParam) * lowFreq;
        // float wc = (freq / (fs * oversample.getOversamplingRatio())) * M_PI;
        filter.setParameters(freq, freq, freq, k);

        int modeChoice = (int) paramQuantities[MODE_PARAM]->getDisplayValue();
        if (modeChoice == 0)
            filter.setFilterTypes(0, 0, 0);
        else if (modeChoice == 1)
            filter.setFilterTypes(0, 1, 0);
        else if (modeChoice == 2)
            filter.setFilterTypes(1, 0, 1);
        else if (modeChoice == 3)
            filter.setFilterTypes(1, 1, 1);
    }

    void process(const ProcessArgs& args) override {
        if(paramDivider.process())
            cookParams();

        float x = inputs[AUDIO_IN].getVoltage();

        auto result = filter.processSample(double (x * 0.1f));
        
        // oversample.upsample(x);
        // float* osBuffer = oversample.getOSBuffer();
        // for(int k = 0; k < oversample.getOversamplingRatio(); k++)
        //     osBuffer[k] = svf.process(osBuffer[k]);
        // float y = oversample.downsample();

        outputs[STAGE1_OUT].setVoltage((float) std::get<0>(result) * 10.0f);
        outputs[STAGE2_OUT].setVoltage((float) std::get<1>(result) * 10.0f);
        outputs[STAGE3_OUT].setVoltage((float) std::get<2>(result) * 10.0f);
	}

    // json_t* dataToJson() override {
	// 	json_t* rootJ = json_object();
	// 	json_object_set_new(rootJ, "osIdx", json_integer(oversample.getOversamplingIndex()));
    //     return rootJ;
    // }

    // void dataFromJson(json_t* rootJ) override {
    //     if(auto* osJson = json_object_get(rootJ, "osIdx"))
    //         oversample.setOversamplingIndex (json_integer_value(osJson));
    // }

    // VariableOversampling<> oversample;

private:
    FullTripoleFilter filter;
    dsp::ClockDivider paramDivider;
};

struct TripoleFilterWidget : ModuleWidget {
	TripoleFilterWidget(TripoleFilter* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WernerFilter.svg")));

        createScrews (*this);

        addInput(createInputCentered<ChowPort>(mm2px(Vec(10.85, 23.75)), module, TripoleFilter::FREQ_IN));
        addInput(createInputCentered<ChowPort>(mm2px(Vec(10.85, 42.75)), module, TripoleFilter::RESONANCE_IN));

        addParam(createParamCentered<ChowKnob>(mm2px(Vec(39.4, 26.75)), module, TripoleFilter::FREQ_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(39.4, 45.75)), module, TripoleFilter::RESONANCE_PARAM));
        addParam(createParamCentered<ChowKnobLarge>(mm2px(Vec(20.5, 64.75)), module, TripoleFilter::MODE_PARAM));

        addParam(createParamCentered<ChowSmallKnob>(mm2px(Vec(24.4, 25.25)), module, TripoleFilter::FREQ_ATTEN_PARAM));
        addParam(createParamCentered<ChowSmallKnob>(mm2px(Vec(24.4, 44.25)), module, TripoleFilter::RESONANCE_ATTEN_PARAM));

        addInput(createInputCentered<ChowPort>(mm2px(Vec(25.4, 97.5)), module, TripoleFilter::AUDIO_IN));
        addOutput(createOutputCentered<ChowPort>(mm2px(Vec(25.4, 115.0)), module, TripoleFilter::STAGE3_OUT));
	}

    // void appendContextMenu(Menu *menu) override {
    //     menu->addChild(new MenuSeparator());
    //     addPubToMenu(menu, "https://dafx2020.mdw.ac.at/proceedings/papers/DAFx2020_paper_70.pdf");
    //     dynamic_cast<Werner*> (module)->oversample.addContextMenu(menu, module);
    // }
};

Model* modelTripoleFilter = createModel<TripoleFilter, TripoleFilterWidget>("TripoleFilter");
