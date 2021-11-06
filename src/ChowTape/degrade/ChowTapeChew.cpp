#include "../../plugin.hpp"
#include "DegradeFilter.h"
#include "Dropout.h"

struct ChowTapeChew : Module {
    enum ParamIds {
        DEPTH_PARAM,
        FREQ_PARAM,
        VAR_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        AUDIO_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        AUDIO_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    ChowTapeChew() {
        config(NUM_PARAMS, NUM_INPUTS,NUM_OUTPUTS, NUM_LIGHTS);

        configInput(AUDIO_INPUT, "Audio");
        configOutput(AUDIO_OUTPUT, "Audio");
        configBypass(AUDIO_INPUT, AUDIO_OUTPUT);

        configParam(DEPTH_PARAM, 0.0f, 1.0f, 0.0f, "Depth", "%", 0.0f, 100.0f);
        configParam(FREQ_PARAM, 0.0f, 1.0f, 0.0f, "Frequency", "%", 0.0f, 100.0f);
        configParam(VAR_PARAM, 0.0f, 1.0f, 0.0f, "Variance", "%", 0.0f, 100.0f);

        onSampleRateChange();
        paramDivider.setDivision(ParamDivide);
    }

    void onSampleRateChange() override {
        sampleRate = getSampleRate();
        filt.reset(sampleRate, int(sampleRate * 0.02f));
        dropout.prepare((double) sampleRate);
        cookParams();
    }

    void onReset() override {
        Module::onReset();
        onSampleRateChange();
    }

    void cookParams() {
        const float highFreq = std::min(22000.0f, 0.49f * sampleRate);
        const float freqChange = highFreq - 5000.0f;

        auto depth = params[DEPTH_PARAM].getValue();
        auto freq = params[FREQ_PARAM].getValue();
        if (freq == 0.0f)
        {
            mix = 0.0f;
            filt.setFreq (highFreq);
        }
        else if (freq == 1.0f)
        {
            mix = 1.0f;
            power = 3.0f * depth;
            filt.setFreq (highFreq - freqChange * depth);
        }
        else if (sampleCounter >= samplesUntilChange)
        {
            sampleCounter = 0;
            isCrinkled = ! isCrinkled;

            if (isCrinkled) // start crinkle
            {
                mix = 1.0f;
                power = (1.0f + 2.0f * random::uniform()) * depth;
                filt.setFreq (highFreq - freqChange * depth);
                samplesUntilChange = getWetTime(freq, depth, params[VAR_PARAM].getValue());
            }
            else // end crinkle
            {
                mix = 0.0f;
                filt.setFreq (highFreq);
                samplesUntilChange = getDryTime(freq, params[VAR_PARAM].getValue());
            }
        }
        else
        {
            power = (1.0f + 2.0f * random::uniform()) * depth;
            if (isCrinkled)
            {
                filt.setFreq (highFreq - freqChange * depth);
                filt.setFreq (highFreq - freqChange * depth);
            }
        }

        dropout.setMix (mix);
        dropout.setPower (1.0f + power);
    }

    void process(const ProcessArgs& args) override {
        if(paramDivider.process())
            cookParams();

        float x = inputs[AUDIO_INPUT].getVoltage() / 10.0f;
        x = dropout.processSample(x);
        x = filt.processSample(x);

        outputs[AUDIO_OUTPUT].setVoltage(x * 10.0f);

        sampleCounter++;
    }

    inline int getDryTime(float freq, float var)
    {
        auto tScale = std::pow(freq, 0.1f);
        auto varScale = std::pow(random::uniform() * 2.0f, var);

        auto minVal = (int) ((1.0f - tScale) * sampleRate * varScale);
        auto maxVal = (int) ((2.0f - 1.99f * tScale) * sampleRate * varScale);
        return (int) random::u32() % (maxVal - minVal) + minVal;
    }

    inline int getWetTime(float freq, float depth, float var)
    {
        auto tScale = std::pow(freq, 0.1f);
        auto start = 0.2f + 0.8f * depth;
        auto end = start - (0.001f + 0.01f * depth);
        auto varScale = std::pow(random::uniform() * 2.0f, var);

        auto minVal = (int) ((1.0f - tScale) * sampleRate * varScale);
        auto maxVal = (int) (((1.0f - tScale) + start - end * tScale) * sampleRate * varScale);
        return (int) random::u32() % (maxVal - minVal) + minVal;
    }

private:
    enum {
        ParamDivide = 64,
    };

    dsp::ClockDivider paramDivider;
    
    float mix = 0.0f;
    float power = 0.0f;
    float sampleRate = 44100.0f;
    int samplesUntilChange = 1000;
    bool isCrinkled = false;
    int sampleCounter = 0;

    Dropout dropout;
    DegradeFilter filt;
};

struct ChowTapeChewWidget : ModuleWidget {
    ChowTapeChewWidget(ChowTapeChew* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChowTapeChew.svg")));
        createScrews(*this);

        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 26.75)), module, ChowTapeChew::DEPTH_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 51.0)), module, ChowTapeChew::FREQ_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 75.5)), module, ChowTapeChew::VAR_PARAM));

        addInput(createInputCentered<ChowPort>(mm2px(Vec(15.25, 97.5)), module, ChowTapeChew::AUDIO_INPUT));
        addOutput(createOutputCentered<ChowPort>(mm2px(Vec(15.25, 115.0)), module, ChowTapeChew::AUDIO_OUTPUT));
    }
};

Model* modelChowTapeChew = createModel<ChowTapeChew, ChowTapeChewWidget>("ChowTapeChew");
