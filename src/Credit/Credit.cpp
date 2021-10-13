#include <atomic>
#include "../plugin.hpp"
#include "FileUtils.hpp"
#include "ModuleWriter.hpp"

struct Credit : Module {
	enum ParamIds {
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
	}

	void process(const ProcessArgs& args) override {}
};

struct CreditWidget : ModuleWidget {
    ModuleWriter mWriter;

    static void saveModules(ModuleWriter& mWriter) {
        auto pathC = file_utils::getChosenFilePath();
        if (pathC == nullptr) {
            return; // fail silently
        }

	    // Append .txt extension if no extension was given.
	    std::string pathStr = pathC.get();
	    if (system::getStem(pathStr) == "") {
	    	pathStr += ".txt";
	    }

	    file_utils::FilePtr file = file_utils::getFilePtr(pathC.get());
	    if (file == nullptr) {
	    	return; // Fail silently
        }

        mWriter(file.get());
    }

	CreditWidget(Credit* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Credit.svg")));
        createScrews(*this);

        struct SaveButton : public PinwheelRust {
            ModuleWriter& mw;
            std::atomic_bool saving;

            SaveButton(ModuleWriter& mw) :
                PinwheelRust(),
                mw(mw) {
                saving.store(true);
            }

            void onButton(const event::Button& e) override {
                if (e.action == GLFW_PRESS)
                    return; // only do action on release!

                saving.store(! saving.load());

                if(saving.load())
                    saveModules(mw);
            }
        };

        auto saveButton = new SaveButton(mWriter);
        saveButton->box.pos = mm2px(Vec(7.62, 105.25));
        saveButton->box.pos = saveButton->box.pos.minus(saveButton->box.size.div(2)); // center
        
        addChild(saveButton);
	}

    void appendContextMenu(Menu *menu) override {
        menu->addChild(new MenuSeparator());
        
        struct URLOptionItem : MenuItem {
            ModuleWriter& mw;

            URLOptionItem(ModuleWriter& mw) : mw(mw) {
                text = "Include plugin URLs";
                rightText = CHECKMARK(mw.writeURLs);
            }

	    	void onAction(const event::Action& e) override {
	    		mw.writeURLs ^= true;
	    	}
	    };

        URLOptionItem* urlOptionItem = new URLOptionItem(mWriter);
        menu->addChild(urlOptionItem);
    }
};


Model* modelCredit = createModel<Credit, CreditWidget>("Credit");