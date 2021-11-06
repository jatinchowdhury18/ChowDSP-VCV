#include "../plugin.hpp"
#include "BBDDelayLine.h"
#include "../shared/iir.hpp"
#include "../shared/SineWave.h"

namespace
{
constexpr float rate1Low = 0.005f;
constexpr float rate1High = 5.0f;
constexpr float rate2Low = 0.5f;
constexpr float rate2High = 40.0f;

constexpr float delay1Ms = 0.6f;
constexpr float delay2Ms = 0.2f;
}

struct ChowChorus : Module {
	enum ParamIds {
        RATE_PARAM,
        DEPTH_PARAM,
        FEEDBACK_PARAM,
        MIX_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
        AUDIO_IN,
		NUM_INPUTS
	};
	enum OutputIds {
        AUDIO_OUT0,
        AUDIO_OUT1,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	ChowChorus() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configInput(AUDIO_IN, "Audio");
        configOutput(AUDIO_OUT0, "Left");
        configOutput(AUDIO_OUT1, "Right");
        configBypass(AUDIO_IN, AUDIO_OUT0);
        configBypass(AUDIO_IN, AUDIO_OUT1);

        configParam(RATE_PARAM, 0.0f, 1.0f, 0.5f, "Rate", "%", 0.0f, 100.0f);
        configParam(DEPTH_PARAM, 0.0f, 1.0f, 0.5f, "Depth", "%", 0.0f, 100.0f);
        configParam(FEEDBACK_PARAM, 0.0f, 1.0f, 0.0f, "Feedback", "%", 0.0f, 100.0f);
        configParam(MIX_PARAM, 0.0f, 1.0f, 0.5f, "Mix", "%", 0.0f, 100.0f);

        onSampleRateChange();
        paramDivider.setDivision(ParamDivide);
	}

    void onSampleRateChange() override {
        float newSampleRate = getSampleRate();

        for(int ch = 0; ch < numChannels; ++ch)
        {
            for (int i = 0; i < delaysPerChannel; ++i)
            {
                delay[ch][i].prepare((double) newSampleRate);
                slowLFOs[ch][i].prepare((double) newSampleRate);
                fastLFOs[ch][i].prepare((double) newSampleRate);
            }

            aaFilter[ch].setParameters(BiquadFilter::LOWPASS, 12000.0f / newSampleRate, 0.7071f, 1.0f);
            dcBlocker[ch].setParameters(BiquadFilter::HIGHPASS, 240.0f / newSampleRate, 0.7071f, 1.0f);
        }
        
        resetProcessing();
    }

    void resetProcessing() {
        constexpr float initialPhases[] = { -M_PI / 3.0f, 0.0f, 0.0f, M_PI / 3.0f };
        for(int ch = 0; ch < numChannels; ++ch)
        {
            for (int i = 0; i < delaysPerChannel; ++i)
            {
                delay[ch][i].prepare((double) getSampleRate());

                auto lfoPhase = initialPhases[ch * delaysPerChannel + i];
                slowLFOs[ch][i].reset(lfoPhase);
                fastLFOs[ch][i].reset(lfoPhase);
            }
            
            aaFilter[ch].reset();
            dcBlocker[ch].reset();
            feedbackState[ch] = 0.0f;
        }

        cookParams(getSampleRate());
    }

    void onReset() override {
        Module::onReset();
        resetProcessing();
    }

    void cookParams(float fs) {
        const auto rateParam = params[RATE_PARAM].getValue();
        const auto slowRate = rate1Low * std::pow(rate1High / rate1Low, rateParam);
        const auto fastRate = rate2Low * std::pow(rate2High / rate2Low, rateParam);

        fbAmount = 0.39f * std::sqrt(params[FEEDBACK_PARAM].getValue());

        for(int ch = 0; ch < numChannels; ++ch)
        {
            for (int i = 0; i < delaysPerChannel; ++i)
            {
                slowLFOs[ch][i].setFrequency(slowRate);
                fastLFOs[ch][i].setFrequency(fastRate);
                delay[ch][i].setFilterFreq(10000.0f);
            }
        }
    }

	void process(const ProcessArgs& args) override {
        if(paramDivider.process())
            cookParams(args.sampleRate);

        const auto fs = args.sampleRate;
        auto slowDepth = 5.0f * delay1Ms * 0.001f * fs * params[DEPTH_PARAM].getValue();
        auto fastDepth = 5.0f * delay2Ms * 0.001f * fs * params[DEPTH_PARAM].getValue();

        auto mixParam = params[MIX_PARAM].getValue();

        for(int ch = 0; ch < numChannels; ++ch)
        {
            auto input = inputs[AUDIO_IN].getVoltage();
            float x = std::tanh(input * 0.075f - feedbackState[ch]);
            float y = 0.0f;

            for(int i = 0; i < delaysPerChannel; ++i)
            {
                float delayAmt = slowDepth * (1.0f + 0.95f * slowLFOs[ch][i].processSample());
                delayAmt += fastDepth * (1.0f + 0.95f * fastLFOs[ch][i].processSample());

                delay[ch][i].setDelayTime((delayAmt + 25.0f) * args.sampleTime);
                y += delay[ch][i].process(x);
            }

            y = aaFilter[ch].process(y);
            feedbackState[ch] = y * fbAmount;
            feedbackState[ch] = dcBlocker[ch].process(feedbackState[ch]);


            outputs[ch].setVoltage(y * 5.0f * mixParam + input * (1.0f - mixParam));
        }
	}

private:
    enum {
        ParamDivide = 64,
    };

    dsp::ClockDivider paramDivider;

    static constexpr int numChannels = 2;
    static constexpr int delaysPerChannel = 2;

    BBDDelayLine<4096> delay[numChannels][delaysPerChannel];

    chowdsp::SineWave<float> slowLFOs[numChannels][delaysPerChannel];
    chowdsp::SineWave<float> fastLFOs[numChannels][delaysPerChannel];

    BiquadFilter aaFilter[2];
    BiquadFilter dcBlocker[2];

    float feedbackState[2] = { 0.0f, 0.0f };
    float fbAmount = 0.0f;
};


struct ChowChorusWidget : ModuleWidget {
	ChowChorusWidget(ChowChorus* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChowChorus.svg")));

        createScrews (*this);

        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 23.0)), module, ChowChorus::RATE_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 43.0)), module, ChowChorus::DEPTH_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 63.0)), module, ChowChorus::FEEDBACK_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 83.0)), module, ChowChorus::MIX_PARAM));

        addInput(createInputCentered<ChowPort>(mm2px(Vec(15.25, 99.0)), module, ChowChorus::AUDIO_IN));
        addOutput(createOutputCentered<ChowPort>(mm2px(Vec(7.625, 115.0)), module, ChowChorus::AUDIO_OUT0));
        addOutput(createOutputCentered<ChowPort>(mm2px(Vec(22.875, 115.0)), module, ChowChorus::AUDIO_OUT1));
	}
};


Model* modelChowChorus = createModel<ChowChorus, ChowChorusWidget>("ChowChorus");