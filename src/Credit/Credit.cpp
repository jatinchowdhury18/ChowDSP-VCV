#include <future>
#include <chrono>
#include <thread>
#include <functional>
#include "../plugin.hpp"
#include "FileUtils.hpp"
#include "ModuleWriter.hpp"

/** Non-blocking version of std::async */
template< class Function, class... Args>
std::future<typename std::result_of<Function(Args...)>::type> myAsync( Function&& f, Args&&... args ) 
{
    typedef typename std::result_of<Function(Args...)>::type R;
    auto bound_task = std::bind(std::forward<Function>(f), std::forward<Args>(args)...);
    std::packaged_task<R()> task(std::move(bound_task));
    auto ret = task.get_future();
    std::thread t(std::move(task));
    t.detach();
    return ret;   
}

struct Credit : Module {
	enum ParamIds {
        SAVE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Credit() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(SAVE_PARAM, 0.0f, 1.0f, 0.0f, "Save");

        saving.store(false);
	}

    void saveModules() {
        auto pathC = file_utils::getChosenFilePath();
        if (pathC == nullptr) {
            return; // fail silently
        }

	    // Append .txt extension if no extension was given.
	    std::string pathStr = pathC.get();
	    if (string::filenameExtension(string::filename(pathStr)) == "") {
	    	pathStr += ".txt";
	    }

	    file_utils::FilePtr file = file_utils::getFilePtr(pathC.get());
	    if (file == nullptr) {
	    	return; // Fail silently
        }

        ModuleWriter mWriter;
        mWriter(file.get());

        saving.store(false);
    }

	void process(const ProcessArgs& args) override {
        if(params[SAVE_PARAM].getValue() && ! saving.load()) {
            saving.store(true);
            myAsync(&Credit::saveModules, this);
        }
	}

private:
    std::atomic_bool saving;
};


struct CreditWidget : ModuleWidget {
	CreditWidget(Credit* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Credit.svg")));
        createScrews(*this);

        addParam(createParamCentered<PinwheelRust>(mm2px(Vec(7.62, 105.25)), module, Credit::SAVE_PARAM));
	}
};


Model* modelCredit = createModel<Credit, CreditWidget>("Credit");