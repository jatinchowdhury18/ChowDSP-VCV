#pragma once
#include <rack.hpp>

using namespace rack;
extern Plugin* pluginInstance;

struct ChowKnob : app::SvgKnob {
    ChowKnob() {
        minAngle = -0.76 * M_PI;
        maxAngle = 0.76 * M_PI;
        shadow->opacity = 0;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Knob.svg")));
    }
};

struct ChowPort : app::SvgPort {
    ChowPort() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Port.svg")));
    }
};

struct PinwheelTeal : app::SvgSwitch {
	PinwheelTeal() {
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/PinwheelTeal_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/PinwheelTeal_1.svg")));
	}
};
