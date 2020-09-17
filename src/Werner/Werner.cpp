#include "../plugin.hpp"
#include "GenSVF.hpp"
#include "../shared/oversampling.hpp"

namespace {
    constexpr float highFreq = 20000.0f;
    constexpr float lowFreq = 20.0f;
    constexpr float highDrive = 10.0f;
    constexpr float lowDrive = 0.1f;

    enum {
        OSRatio = 2,
        ParamDivide = 16,
    };
}

struct Werner : Module {
    enum ParamIds {
        FREQ_PARAM,
        FB_PARAM,
        DAMPING_PARAM,
        DRIVE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
        AUDIO_IN,
        FREQ_IN,
        FB_IN,
        DAMPING_IN,
        DRIVE_IN,
		NUM_INPUTS
	};
	enum OutputIds {
        AUDIO_OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

    Werner() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(FREQ_PARAM, 0.0f, 1.0f, 0.5f, "Freq.", " Hz", highFreq / lowFreq, lowFreq);
        configParam(DAMPING_PARAM, 0.25f, 1.25f, 0.5f, "Damp");
        configParam(FB_PARAM, 0.0f, 0.95f, 0.5f, "Feedback");
        configParam(DRIVE_PARAM, 0.0f, 1.0f, 0.0f, "Drive", "", highDrive / lowDrive, lowDrive);

        svf.reset();
        onSampleRateChange();
        paramDivider.setDivision(ParamDivide);
    }

    void onReset() override {
        Module::onReset();
        svf.reset();
    }

    void onSampleRateChange() override {
        float newSampleRate = getSampleRate();
        oversample.reset(newSampleRate);
        cookParams(newSampleRate);
    }

    void cookParams(float fs) {
        // calc filter params
        float r = params[DAMPING_PARAM].getValue() + inputs[DAMPING_IN].getVoltage() / 10.0f;
        r = clamp(r, 0.25f, 1.25f);

        float k = params[FB_PARAM].getValue() + inputs[FB_IN].getVoltage() / 10.0f;
        k = clamp(k, 0.0f, 1.0f);

        float freqParam = params[FREQ_PARAM].getValue() + inputs[FREQ_IN].getVoltage() / 10.0f;
        freqParam = clamp(freqParam, 0.0f, 1.0f);
        auto freq = pow(highFreq / lowFreq, freqParam) * lowFreq;
        float wc = (freq / fs) * M_PI_2;
        svf.calcCoefs(r, k, wc);

        // calc drive param
        float driveParam = params[DRIVE_PARAM].getValue() + inputs[DRIVE_IN].getVoltage() / 10.0f;
        driveParam = clamp(driveParam, 0.0f, 1.0f);
        float drive = pow(highDrive / lowDrive, pow(driveParam, 0.33f)) * lowDrive;
        svf.setDrive(drive);
    }

    void process(const ProcessArgs& args) override {
        if(paramDivider.process())
            cookParams(args.sampleRate);

        float x = inputs[AUDIO_IN].getVoltage();
        
        oversample.upsample(x);
        for(int k = 0; k < OSRatio; k++)
            oversample.osBuffer[k] = svf.process(oversample.osBuffer[k]);
        float y = oversample.downsample();

        outputs[AUDIO_OUT].setVoltage(y);
	}

private:
    GeneralSVF svf;
    OversampledProcess<OSRatio> oversample;
    dsp::ClockDivider paramDivider;
};

struct WernerWidget : ModuleWidget {
	WernerWidget(Werner* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WernerFilter.svg")));

        createScrews (*this);

        addInput(createInputCentered<ChowPort>(mm2px(Vec(10.85, 23.75)), module, Werner::FREQ_IN));
        addInput(createInputCentered<ChowPort>(mm2px(Vec(10.85, 42.75)), module, Werner::FB_IN));
        addInput(createInputCentered<ChowPort>(mm2px(Vec(10.85, 61.75)), module, Werner::DAMPING_IN));
        addInput(createInputCentered<ChowPort>(mm2px(Vec(10.85, 81.0)),  module, Werner::DRIVE_IN));

        addParam(createParamCentered<ChowKnob>(mm2px(Vec(29.9, 26.75)), module, Werner::FREQ_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(29.9, 45.75)), module, Werner::FB_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(29.9, 64.75)), module, Werner::DAMPING_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(29.9, 84.0)),  module, Werner::DRIVE_PARAM));

        addInput(createInputCentered<ChowPort>(mm2px(Vec(20.5, 97.5)), module, Werner::AUDIO_IN));
        addOutput(createOutputCentered<ChowPort>(mm2px(Vec(20.5, 115.0)), module, Werner::AUDIO_OUT));
	}

    void appendContextMenu(Menu *menu) override {
        addPubToMenu(menu, "https://dafx2020.mdw.ac.at/proceedings/papers/DAFx2020_paper_70.pdf");
    }
};

Model* modelWerner = createModel<Werner, WernerWidget>("Werner");
