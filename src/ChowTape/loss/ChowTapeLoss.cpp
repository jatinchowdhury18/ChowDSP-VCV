#include "../../plugin.hpp"
#include "FIRFilter.h"
#include "../../shared/iir.hpp"

namespace
{
constexpr float speedBase = 25.0f / 4.0f;
constexpr float speedMult = 28.0f / 3.0f;
constexpr float speedOff = -25.0f / 3.0f;
inline float getSpeed(float param) {
    return std::pow(speedBase, param) * speedMult + speedOff;
}

constexpr float spaceBase = 10000.0f / 9801.0f;
constexpr float spaceMult = 9801.0f / 10.0f;
constexpr float spaceOff = -980.0f;
inline float getSpacing(float param) {
    return std::pow(spaceBase, param) * spaceMult + spaceOff;
}

constexpr float thickBase = 122500.0f / 22201.0f;
constexpr float thickMult = 22201.0f / 2010.0f;
constexpr float thickOff = -2200.0f / 201.0f;
inline float getThickness(float param) {
    return std::pow(thickBase, param) * thickMult + thickOff;
}

constexpr float gapBase = 1600.0f / 81.0f;
constexpr float gapMult = 81.0f / 31.0f;
constexpr float gapOff = -50.0f / 31.0f;
inline float getGap(float param) {
    return std::pow(gapBase, param) * gapMult + gapOff;
}
}

struct ChowTapeLoss : Module {
    enum ParamIds {
        GAP_PARAM,
        THICK_PARAM,
        SPACE_PARAM,
        SPEED_PARAM,
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

    ChowTapeLoss() {
        config(NUM_PARAMS, NUM_INPUTS,NUM_OUTPUTS, NUM_LIGHTS);

        configInput(AUDIO_INPUT, "Audio");
        configOutput(AUDIO_OUTPUT, "Audio");
        configBypass(AUDIO_INPUT, AUDIO_OUTPUT);

        configParam(GAP_PARAM, 0.0f, 1.0f, 0.5f, "Gap", " µ", gapBase, gapMult, gapOff);
        configParam(THICK_PARAM, 0.0f, 1.0f, 0.5f, "Thickness", " µ", thickBase, thickMult, thickOff);
        configParam(SPACE_PARAM, 0.0f, 1.0f, 0.5f, "Spacing", " µ", spaceBase, spaceMult, spaceOff);
        configParam(SPEED_PARAM, 0.0f, 1.0f, 0.5f, "Speed", " ips", speedBase, speedMult, speedOff);

        onSampleRateChange();
        paramDivider.setDivision(ParamDivide);
    }

    void onSampleRateChange() override {
        fs = getSampleRate();

        fsFactor = (float) fs / 44100.0f;
        curOrder = int (order * fsFactor);
        currentCoefs.resize (curOrder, 0.0f);
        Hcoefs.resize (curOrder, 0.0f);
        calcCoefs();

        filter.reset(new FIRFilter (curOrder));
        filter->setCoefs(currentCoefs.data());
        filter->reset();

        prevSpeed = params[SPEED_PARAM].getValue();
        prevSpacing = params[SPACE_PARAM].getValue();
        prevThickness = params[THICK_PARAM].getValue();
        prevGap = params[GAP_PARAM].getValue();
    }

    void onReset() override {
        Module::onReset();
        
        filter->reset();
        headBumpFilter.reset();
    }

    void calcCoefs()
    {
        const auto speed = getSpeed(params[SPEED_PARAM].getValue());
        const auto thickness = getThickness(params[THICK_PARAM].getValue());
        const auto gap = getGap(params[GAP_PARAM].getValue());
        const auto spacing = getSpacing(params[SPACE_PARAM].getValue());

        // Set freq domain multipliers
        binWidth = fs / (float) curOrder;
        auto H = Hcoefs.data();
        for (int k = 0; k < curOrder / 2; k++)
        {
            const auto freq = (float) k * binWidth;
            const auto waveNumber = 2.0f * M_PI * std::max(freq, 20.0f) / (speed * 0.0254f);
            const auto thickTimesK = waveNumber * (thickness * (float) 1.0e-6);
            const auto kGapOverTwo = waveNumber * (gap * (float) 1.0e-6) / 2.0f;

            H[k] = expf (-waveNumber * (spacing * (float) 1.0e-6)); // Spacing loss
            H[k] *= (1.0f - expf (-thickTimesK)) / thickTimesK; // Thickness loss
            H[k] *= sinf (kGapOverTwo) / kGapOverTwo; // Gap loss
            H[curOrder - k - 1] = H[k];
        }

        // Create time domain filter signal
        auto h = currentCoefs.data();
        for (int n = 0; n < curOrder / 2; n++)
        {
            const auto idx = (size_t) curOrder / 2 + (size_t) n;
            for (int k = 0; k < curOrder; k++)
                h[idx] += Hcoefs[k] * cosf (2.0f * M_PI * (float) k * (float) n / (float) curOrder);

            h[idx] /= (float) curOrder;
            h[curOrder / 2 - n] = h[idx];
        }

        // compute head bump filters
        calcHeadBumpFilter(speed, gap * 1.0e-6f, fs);
    }

    void calcHeadBumpFilter(float speedIps, float gapMeters, float fs)
    {
        auto bumpFreq = speedIps * 0.0254f / (gapMeters * 500.0f);
        auto gain = std::max(1.5f * (1000.0f - std::abs(bumpFreq - 100.0f)) / 1000.0f, 1.0f);
        headBumpFilter.setParameters(BiquadFilter::PEAK, bumpFreq / fs, 2.0f, gain);
    }

    void cookParams() {
        auto speedParam = params[SPEED_PARAM].getValue();
        auto spaceParam = params[SPACE_PARAM].getValue();
        auto thickParam = params[THICK_PARAM].getValue();
        auto gapParam = params[GAP_PARAM].getValue();

        if(speedParam == prevSpeed && spaceParam == prevSpacing
            && thickParam == prevThickness && gapParam == prevGap)
            return;

        calcCoefs();
        filter->setCoefs(currentCoefs.data());

        prevSpeed = speedParam;
        prevSpacing = spaceParam;
        prevThickness = thickParam;
        prevGap = gapParam;
    }

    void process(const ProcessArgs& args) override {
        if(paramDivider.process())
            cookParams();

        float x = inputs[AUDIO_INPUT].getVoltage();
        x = filter->process(x);
        x = headBumpFilter.process(x);
        outputs[AUDIO_OUTPUT].setVoltage(x);
    }

private:
    enum {
        ParamDivide = 128,
    };

    dsp::ClockDivider paramDivider;


    float prevSpeed = 0.5f;
    float prevSpacing = 0.5f;
    float prevThickness = 0.5f;
    float prevGap = 0.5f;

    float fs = 44100.0f;
    float fsFactor = 1.0f;
    float binWidth = fs / 100.0f;

    const int order = 64;
    int curOrder = order;
    std::vector<float> currentCoefs;
    std::vector<float> Hcoefs;

    std::unique_ptr<FIRFilter> filter;
    BiquadFilter headBumpFilter;
};

struct ChowTapeLossWidget : ModuleWidget {
    ChowTapeLossWidget(ChowTapeLoss* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChowTapeLoss.svg")));
        createScrews(*this);

        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 23.0)), module, ChowTapeLoss::GAP_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 43.0)), module, ChowTapeLoss::THICK_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 63.0)), module, ChowTapeLoss::SPACE_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 83.0)), module, ChowTapeLoss::SPEED_PARAM));

        addInput(createInputCentered<ChowPort>(mm2px(Vec(15.25, 99.0)), module, ChowTapeLoss::AUDIO_INPUT));
        addOutput(createOutputCentered<ChowPort>(mm2px(Vec(15.25, 115.0)), module, ChowTapeLoss::AUDIO_OUTPUT));
    }
};

Model* modelChowTapeLoss = createModel<ChowTapeLoss, ChowTapeLossWidget>("ChowTapeLoss");
