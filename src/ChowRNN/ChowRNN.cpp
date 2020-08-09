#include "plugin.hpp"

#include <memory>
#include <MLUtils/Model.h>

#include "LayerRandomiser.hpp"

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

        rando.zeroDenseBias(dynamic_cast<MLUtils::Dense<float>*> (model.layers.back()));
	}

    void randomiseModel() {
        if(auto denseLayer = dynamic_cast<MLUtils::Dense<float>*> (model.layers[0])) {
            rando.randomDenseWeights(denseLayer);
            rando.randomDenseBias(denseLayer);
        }

        if(auto gruLayer = dynamic_cast<MLUtils::GRULayer<float>*> (model.layers[2]))
            rando.randomGRU(gruLayer);

        if(auto denseLayer = dynamic_cast<MLUtils::Dense<float>*> (model.layers[3]))
            rando.randomDenseWeights(denseLayer);
    }

	void process(const ProcessArgs& args) override {
        // randomise if needed
        if(params[RANDOM_PARAM].getValue()) {
            randomiseModel();
        }

        // load RNN inputs
        float input[NDims];
        for(int i = 0; i < NUM_INPUTS; ++i)
            input[i] = inputs[i].getVoltage();

        // process RNN
        float y = model.forward(input);

        // apply DC blocker
        dcBlocker.setParameters(dsp::BiquadFilter::HIGHPASS, 30.0f / args.sampleRate, M_SQRT1_2, 1.0f);
        y = dcBlocker.process (y);

        outputs[OUT1].setVoltage(y);
	}

private:
    enum {
        NDims = 4,
    };

    MLUtils::Model<float> model { NDims };
    LayerRandomiser rando;
    dsp::BiquadFilter dcBlocker;
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