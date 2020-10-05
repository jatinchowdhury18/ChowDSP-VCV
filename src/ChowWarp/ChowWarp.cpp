#include "ChowWarp.hpp"
#include "Mappings.hpp"
#include "../shared/ChoiceQuantity.hpp"

namespace {
    constexpr float highFreq = 1000.0f;
    constexpr float lowFreq = 5.0f;
}

ChowWarp::ChowWarp() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(CUTOFF_PARAM, 0.0f, 1.0f, 0.5f, "Cutoff", " Hz", highFreq / lowFreq, lowFreq);
    configParam(HEAT_PARAM, 0.0f, 1.0f, 0.5f, "Heat");
    configParam(WIDTH_PARAM, 0.0f, 1.0f, 0.5f, "Width");
    configParam(DRIVE_PARAM, 0.0f, 1.0f, 0.0f, "Drive");
    configParam<ChoiceQuantity>(MODE_PARAM, 0.0f, (float) ChowWarpMappings::NumMappings, 0.0f, "Mode");

    paramMapSets.push_back(ChowWarpMappings::mapping1(*this, warpFilter));
    paramMapSets.push_back(ChowWarpMappings::mapping2(*this, warpFilter));
}

void ChowWarp::onSampleRateChange() {
    warpFilter.onSampleRateChange();
}

void ChowWarp::process(const ProcessArgs& args) {
    int mapChoice = (int) paramQuantities[MODE_PARAM]->getDisplayValue();
    ParamMap::applySet(paramMapSets[mapChoice]);

    warpFilter.inputs[0].setVoltage(inputs[AUDIO_IN].getVoltage());
    warpFilter.process(args);
    outputs[AUDIO_OUT].setVoltage(10.0f * warpFilter.outputs[0].getVoltage());
}

struct ChowWarpWidget : ModuleWidget {
	ChowWarpWidget(ChowWarp* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChowWarp.svg")));
        createScrews (*this);

        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25,  5.0)), module, ChowWarp::CUTOFF_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 20.0)), module, ChowWarp::HEAT_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 35.0)), module, ChowWarp::WIDTH_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 50.0)), module, ChowWarp::DRIVE_PARAM));
        addParam(createParamCentered<ChowKnob>(mm2px(Vec(15.25, 65.0)), module, ChowWarp::MODE_PARAM));

        addInput(createInputCentered<ChowPort>(mm2px(Vec(15.25, 99.0)), module, ChowWarp::AUDIO_IN));
        addOutput(createOutputCentered<ChowPort>(mm2px(Vec(15.25, 115.0)), module, ChowWarp::AUDIO_OUT));
	}
};


Model* modelChowWarp = createModel<ChowWarp, ChowWarpWidget>("ChowWarp");