#include "plugin.hpp"

#include <memory>
#include <MLUtils/Model.h>

struct ChowRNN : Module {
	enum ParamIds {
        RANDOM_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
        IN1,
        IN2,
        IN3,
        IN4,
		NUM_INPUTS
	};
	enum OutputIds {
        OUT1,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	ChowRNN() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(RANDOM_PARAM, 0.0f, 1.0f, 0.0f, "Randomise");

        model.addLayer(new MLUtils::Dense<float> (NDims, NDims));
        model.addLayer(new MLUtils::TanhActivation<float> (NDims));
        model.addLayer(new MLUtils::GRULayer<float> (NDims, NDims));
        model.addLayer(new MLUtils::Dense<float> (NDims, 1));

        model.reset();
	}

	void process(const ProcessArgs& args) override {
        float input[NDims];

        for(int i = 0; i < NUM_INPUTS; ++i)
            input[i] = inputs[i].getVoltage();

        outputs[OUT1].setVoltage(model.forward(input));
	}

private:
    enum {
        NDims = 4,
    };

    MLUtils::Model<float> model { NDims };
};


struct ChowRNNWidget : ModuleWidget {
	ChowRNNWidget(ChowRNN* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChowRNN.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<LEDBezel>(mm2px(Vec(22.875, 83.0)), module, ChowRNN::RANDOM_PARAM));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.625, 33.0)), module, ChowRNN::IN1));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.625, 53.0)), module, ChowRNN::IN2));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.625, 73.0)), module, ChowRNN::IN3));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.625, 93.0)), module, ChowRNN::IN4));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.875, 63.0)), module, ChowRNN::OUT1));
	}
};


Model* modelChowRNN = createModel<ChowRNN, ChowRNNWidget>("ChowRNN");