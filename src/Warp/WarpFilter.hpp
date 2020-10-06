#pragma once

#include "../plugin.hpp"
#include "../shared/oversampling.hpp"
#include "../shared/nl_biquad.hpp"
#include "NewtonRaphson.hpp"

struct WarpFilter : Module {
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

	WarpFilter();

    void onSampleRateChange() override;
    void cookParams(float sampleRate) noexcept;
	void process(const ProcessArgs& args) override;

private:
    inline float processOS(float x) noexcept;

    enum {
        OSRatio = 2,
    };

    OversampledProcess<OSRatio> oversample;
    NLBiquad filter;
    DFLFilter<4> nrSolver;
};
