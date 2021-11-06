#include "../../plugin.hpp"
#include "../../shared/LevelDetector.hpp"

namespace
{
constexpr float attackBase = 160000.0f / 9801.0f;
constexpr float attackMult = 9801.0f / 3010.0f;
constexpr float attackOff = -950.0f / 301.0f;

constexpr float releaseBase = 100.0f;
constexpr float releaseMult = 10.0f;
}

struct ChowTapeCompression : Module {
    enum ParamIds {
        AMOUNT_PARAM,
        ATTACK_PARAM,
        RELEASE_PARAM,
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

    ChowTapeCompression() {
        config(NUM_PARAMS, NUM_INPUTS,NUM_OUTPUTS, NUM_LIGHTS);

        configInput(AUDIO_INPUT, "Audio");
        configOutput(AUDIO_OUTPUT, "Audio");
        configBypass(AUDIO_INPUT, AUDIO_OUTPUT);

        configParam(AMOUNT_PARAM, 0.0f, 9.0f, 0.0f, "Amount", " dB");
        configParam(ATTACK_PARAM, 0.0f, 1.0f, 0.5f, "Attack", " ms", attackBase, attackMult, attackOff);
        configParam(RELEASE_PARAM, 0.0f, 1.0f, 0.5f, "Release", " ms", releaseBase, releaseMult);

        onSampleRateChange();
        paramDivider.setDivision(ParamDivide);
    }

    inline float getAttackTime() {
        return std::pow(attackBase, params[ATTACK_PARAM].getValue()) * attackMult + attackOff;
    }
    
    inline float getReleaseTime() {
        return std::pow(releaseBase, params[RELEASE_PARAM].getValue()) * releaseMult;
    }

    inline float compressionDB (float xDB, float dbPlus)
    {
        auto window = 2.0f * dbPlus;

        if (dbPlus <= 0.0f || xDB < -window)
            return dbPlus;

        return std::log (xDB + window + 1.0f) - dbPlus - xDB;
    }

    void onSampleRateChange() override {
        float newSampleRate = getSampleRate();
        slewLimiter.prepare(newSampleRate);
        cookParams();
    }

    void onReset() override {
        Module::onReset();
        slewLimiter.reset();
    }

    void cookParams() {
        slewLimiter.setParameters(getAttackTime(), getReleaseTime());
    }

    void process(const ProcessArgs& args) override {
        if(paramDivider.process())
            cookParams();

        float x = inputs[AUDIO_INPUT].getVoltage() / 5.0f;
        auto xDB = dsp::amplitudeToDb(std::abs(x));
        auto compDB = compressionDB(xDB, params[AMOUNT_PARAM].getValue());
        auto compGain = dsp::dbToAmplitude(compDB);
        compGain = std::min(compGain, slewLimiter.processSample(compGain));

        outputs[AUDIO_OUTPUT].setVoltage(compGain * x * 5.0f);
    }

private:
    enum {
        ParamDivide = 32,
    };

    dsp::ClockDivider paramDivider;
    LevelDetector<float> slewLimiter;
};

struct ChowTapeCompressionWidget : ModuleWidget {
    ChowTapeCompressionWidget(ChowTapeCompression* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChowTapeCompression.svg")));
        createScrews(*this);

        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 26.75)), module, ChowTapeCompression::AMOUNT_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 51.0)), module, ChowTapeCompression::ATTACK_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 75.5)), module, ChowTapeCompression::RELEASE_PARAM));

        addInput(createInputCentered<ChowPort>(mm2px(Vec(15.25, 97.5)), module, ChowTapeCompression::AUDIO_INPUT));
        addOutput(createOutputCentered<ChowPort>(mm2px(Vec(15.25, 115.0)), module, ChowTapeCompression::AUDIO_OUTPUT));
    }
};

Model* modelChowTapeCompression = createModel<ChowTapeCompression, ChowTapeCompressionWidget>("ChowTapeComp");
