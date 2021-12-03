#include "Warp.hpp"
#include "Mappings.hpp"
#include "../shared/ChoiceQuantity.hpp"

namespace {
    constexpr float highFreq = 1000.0f;
    constexpr float lowFreq = 5.0f;
}

Warp::Warp() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configInput(AUDIO_IN, "Audio");
    configOutput(AUDIO_OUT, "Audio");
    configBypass(AUDIO_IN, AUDIO_OUT);

    configParam(CUTOFF_PARAM, 0.0f, 1.0f, 0.5f, "Cutoff", " Hz", highFreq / lowFreq, lowFreq);
    configParam(HEAT_PARAM, 0.0f, 1.0f, 0.5f, "Heat");
    configParam(WIDTH_PARAM, 0.0f, 1.0f, 0.5f, "Width");
    configParam(DRIVE_PARAM, 0.0f, 1.0f, 0.0f, "Drive");
    configParam<ChoiceQuantity>(MODE_PARAM, 0.0f, (float) WarpMappings::NumMappings, 0.0f, "Mode");

    paramMapSets.push_back(WarpMappings::mapping1(*this, warpFilter));
    paramMapSets.push_back(WarpMappings::mapping2(*this, warpFilter));
    paramMapSets.push_back(WarpMappings::mapping3(*this, warpFilter));

    paramDivider.setDivision(ParamDivide);
}

void Warp::onSampleRateChange() {
    warpFilter.onSampleRateChange();
}

void Warp::onReset() {
    Module::onReset();
    warpFilter.onSampleRateChange();
}

void Warp::cookParams(float sampleRate) noexcept {
#if CHOWDSP_BENCH
    int mapChoice = 0;
#else
    int mapChoice = (int) paramQuantities[MODE_PARAM]->getDisplayValue();
#endif

    if(mapChoice != prevMapChoice && fadeCounter == 0) {
        fadeCounter = FadeCount;
        prevMapChoice = mapChoice;
    }

    ParamMap::applySet(paramMapSets[mapChoice]);
    warpFilter.cookParams(sampleRate);
}

void Warp::process(const ProcessArgs& args) {
    if(paramDivider.process())
        cookParams(args.sampleRate);

    warpFilter.inputs[0].setVoltage(inputs[AUDIO_IN].getVoltage());
    warpFilter.process(args);

    float outGain = 10.0f;
    if(fadeCounter > 0) {
        fadeCounter--;
        outGain *= 1.0f - ((float) fadeCounter / (float) FadeCount);
    }

    outputs[AUDIO_OUT].setVoltage(outGain * warpFilter.outputs[0].getVoltage());
}

json_t* Warp::dataToJson() {
	json_t* rootJ = json_object();
	json_object_set_new(rootJ, "osIdx", json_integer(warpFilter.oversample.getOversamplingIndex()));
    return rootJ;
}

void Warp::dataFromJson(json_t* rootJ) {
    if(auto* osJson = json_object_get(rootJ, "osIdx"))
        warpFilter.oversample.setOversamplingIndex (json_integer_value(osJson));
}

struct WarpWidget : ModuleWidget {
    ChowLabel* modeLabel;

	WarpWidget(Warp* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Warp.svg")));
        createScrews (*this);

        constexpr double x1 = 10.85;
        constexpr double x2 = 29.9;

        addParam(createParamCentered<ChowKnob>(mm2px(Vec(x1, 26.75)), module, Warp::CUTOFF_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(x1, 51.0)), module, Warp::HEAT_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(x2, 26.75)), module, Warp::WIDTH_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(x2, 51.0)), module, Warp::DRIVE_PARAM));
        addParam(createParamCentered<ChowKnobLarge>(mm2px(Vec(20.5, 76.0)), module, Warp::MODE_PARAM));

        modeLabel = createWidget<ChowLabel>(mm2px(Vec(20.5, 90.0)));
        modeLabel->box.size = mm2px(Vec(20.0, 15.0));
        modeLabel->fontSize = 15.0f;
        modeLabel->alignment = Label::Alignment::CENTER_ALIGNMENT;
        modeLabel->fontPath = asset::plugin(pluginInstance, "res/fonts/RobotoCondensed-Bold.ttf");
        if(! module)
            modeLabel->text = "Mode: 0";
        addChild(modeLabel);

        addInput(createInputCentered<ChowPort>(mm2px(Vec(20.5, 99.0)), module, Warp::AUDIO_IN));
        addOutput(createOutputCentered<ChowPort>(mm2px(Vec(20.5, 115.0)), module, Warp::AUDIO_OUT));
	}

    void appendContextMenu(Menu *menu) override {
        menu->addChild(new MenuSeparator());
        dynamic_cast<Warp*> (module)->warpFilter.oversample.addContextMenu(menu, module);
    }

    void step() override {
        ModuleWidget::step();
        if (auto mod = dynamic_cast<Warp*> (module)) {
            int mapChoice = (int) mod->paramQuantities[Warp::MODE_PARAM]->getDisplayValue();
            modeLabel->text = "Mode: " + std::to_string(mapChoice);
        }
    }
};


Model* modelWarp = createModel<Warp, WarpWidget>("Warp");