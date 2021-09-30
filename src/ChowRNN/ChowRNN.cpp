#include "../plugin.hpp"

#include <memory>
#include <MLUtils/Model.h>

#include "../shared/iir.hpp"
#include "LayerRandomiser.hpp"
#include "LayerJSON.hpp"

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

        configInput(IN1, "Channel 1");
        configInput(IN2, "Channel 2");
        configInput(IN3, "Channel 3");
        configInput(IN4, "Channel 4");
        configOutput(OUT1, "Audio");

        configParam(RANDOM_PARAM, 0.0f, 1.0f, 0.0f, "Randomise");

        // model architecture: input -> Dense(4) -> Tanh Activation(4) -> GRU(4) -> Dense(1)
        model.addLayer(new MLUtils::Dense<float> (NDims, NDims));
        model.addLayer(new MLUtils::TanhActivation<float> (NDims));
        model.addLayer(new MLUtils::GRULayer<float> (NDims, NDims));
        model.addLayer(new MLUtils::Dense<float> (NDims, 1));

        model.reset();

        // no bias on the output layer, since we're going to use a DC blocker anyway
        rando.zeroDenseBias(dynamic_cast<MLUtils::Dense<float>*> (model.layers.back()));
	}

    // randomise model weights
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

        // Unstable RNN weights??
        if (std::isnan(y))
        {
            y = 0.0f;
            model.reset();
        }

        // apply DC blocker
        dcBlocker.setParameters(BiquadFilter::HIGHPASS, 30.0f / args.sampleRate, M_SQRT1_2, 1.0f);
        y = dcBlocker.process(y);

        // makeup gain
        int numConnected = 0;
        for(auto in : inputs)
            if(in.isConnected()) numConnected++;
        float makeupGain = 4.0f / (float) std::max(numConnected, 1);

        outputs[OUT1].setVoltage(y * makeupGain);
	}

    void onReset() override {
        model.reset();
    }

    // save model weights to JSON
    json_t* dataToJson() override {
        json_t* rootJ = json_object();
        
        if(auto denseLayer = dynamic_cast<MLUtils::Dense<float>*> (model.layers[0])) {
            auto dense1J = LayerJson::DenseToJson(denseLayer);
            json_object_set_new(rootJ, "dense1", dense1J);
        }

        if(auto gruLayer = dynamic_cast<MLUtils::GRULayer<float>*> (model.layers[2])) {
            auto gruJ = LayerJson::GruToJson(gruLayer);
            json_object_set_new(rootJ, "gru", gruJ);
        }

        if(auto denseLayer = dynamic_cast<MLUtils::Dense<float>*> (model.layers[3])) {
            auto denseOutJ = LayerJson::DenseToJson(denseLayer);
            json_object_set_new(rootJ, "denseOut", denseOutJ);
        }

        return rootJ;
    }

    // load model weights from JSON
    void dataFromJson(json_t* json) override {
        json_t* dense1J = json_object_get(json, "dense1");
        if(dense1J) {
            if(auto denseLayer = dynamic_cast<MLUtils::Dense<float>*> (model.layers[0]))
                LayerJson::JsonToDense(denseLayer, dense1J);
        }

        json_t* gruJ = json_object_get(json, "gru");
        if(gruJ) {
            if(auto gruLayer = dynamic_cast<MLUtils::GRULayer<float>*> (model.layers[2]))
                LayerJson::JsonToGru(gruLayer, gruJ);
        }

        json_t* denseOutJ = json_object_get(json, "denseOut");
        if(denseOutJ) {
            if(auto denseLayer = dynamic_cast<MLUtils::Dense<float>*> (model.layers[3]))
                LayerJson::JsonToDense(denseLayer, denseOutJ);
        }
    }

private:
    enum {
        NDims = 4,
    };

    MLUtils::Model<float> model { NDims };
    LayerRandomiser rando;
    BiquadFilter dcBlocker;
};


struct ChowRNNWidget : ModuleWidget {
	ChowRNNWidget(ChowRNN* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChowRNN.svg")));
        createScrews(*this);

        addParam(createParamCentered<PinwheelTeal>(mm2px(Vec(15.25, 97.25)), module, ChowRNN::RANDOM_PARAM));

        addInput(createInputCentered<ChowPort>(mm2px(Vec(15.25, 23.75)), module, ChowRNN::IN1));
        addInput(createInputCentered<ChowPort>(mm2px(Vec(15.25, 40.5)), module, ChowRNN::IN2));
        addInput(createInputCentered<ChowPort>(mm2px(Vec(15.25, 57.5)), module, ChowRNN::IN3));
        addInput(createInputCentered<ChowPort>(mm2px(Vec(15.25, 74.5)), module, ChowRNN::IN4));
		addOutput(createOutputCentered<ChowPort>(mm2px(Vec(15.15, 115.0)), module, ChowRNN::OUT1));
	}
};


Model* modelChowRNN = createModel<ChowRNN, ChowRNNWidget>("ChowRNN");