#pragma once

#include "../plugin.hpp"
#include "WarpFilter.hpp"
#include "../shared/ParamMap.hpp"

struct Warp : Module {
	enum ParamIds {
        CUTOFF_PARAM,
        HEAT_PARAM,
        WIDTH_PARAM,
        DRIVE_PARAM,
        MODE_PARAM,
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

	Warp();

    void onSampleRateChange() override;
    void onReset() override;
	void process(const ProcessArgs& args) override;

    json_t* dataToJson() override;
    void dataFromJson(json_t* rootJ) override;

    WarpFilter warpFilter;

private:
    inline void cookParams(float sampleRate) noexcept;

    enum {
        ParamDivide = 16,
        FadeCount = 2048,
    };

    std::vector<ParamMap::ParamMapSet> paramMapSets;
    dsp::ClockDivider paramDivider;

    size_t fadeCounter = 0;
    int prevMapChoice = 0;
};
