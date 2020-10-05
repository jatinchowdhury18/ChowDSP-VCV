#include "Warp.hpp"
#include "Mappings.hpp"
#include "../shared/ChoiceQuantity.hpp"

namespace {
    constexpr float highFreq = 1000.0f;
    constexpr float lowFreq = 5.0f;
}

Warp::Warp() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(CUTOFF_PARAM, 0.0f, 1.0f, 0.5f, "Cutoff", " Hz", highFreq / lowFreq, lowFreq);
    configParam(HEAT_PARAM, 0.0f, 1.0f, 0.5f, "Heat");
    configParam(WIDTH_PARAM, 0.0f, 1.0f, 0.5f, "Width");
    configParam(DRIVE_PARAM, 0.0f, 1.0f, 0.0f, "Drive");
    configParam<ChoiceQuantity>(MODE_PARAM, 0.0f, (float) WarpMappings::NumMappings, 0.0f, "Mode");

    paramMapSets.push_back(WarpMappings::mapping1(*this, warpFilter));
    paramMapSets.push_back(WarpMappings::mapping2(*this, warpFilter));
}

void Warp::onSampleRateChange() {
    warpFilter.onSampleRateChange();
}

void Warp::process(const ProcessArgs& args) {
    int mapChoice = (int) paramQuantities[MODE_PARAM]->getDisplayValue();
    ParamMap::applySet(paramMapSets[mapChoice]);

    warpFilter.inputs[0].setVoltage(inputs[AUDIO_IN].getVoltage());
    warpFilter.process(args);
    outputs[AUDIO_OUT].setVoltage(10.0f * warpFilter.outputs[0].getVoltage());
}

struct WarpWidget : ModuleWidget {
	WarpWidget(Warp* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Warp.svg")));
        createScrews (*this);

        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25,  5.0)), module, Warp::CUTOFF_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 20.0)), module, Warp::HEAT_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 35.0)), module, Warp::WIDTH_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 50.0)), module, Warp::DRIVE_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 65.0)), module, Warp::MODE_PARAM));

        addInput(createInputCentered<ChowPort>(mm2px(Vec(15.25, 99.0)), module, Warp::AUDIO_IN));
        addOutput(createOutputCentered<ChowPort>(mm2px(Vec(15.25, 115.0)), module, Warp::AUDIO_OUT));
	}
};


Model* modelWarp = createModel<Warp, WarpWidget>("Warp");