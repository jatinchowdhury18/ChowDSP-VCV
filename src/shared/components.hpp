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

struct ChowKnobLarge : app::SvgKnob {
    ChowKnobLarge() {
        minAngle = -0.76 * M_PI;
        maxAngle = 0.76 * M_PI;
        shadow->opacity = 0;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/KnobLarge.svg")));
    }
};

struct ChowSmallKnob : app::SvgKnob {
    ChowSmallKnob() {
        minAngle = -0.76 * M_PI;
        maxAngle = 0.76 * M_PI;
        shadow->opacity = 0;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/SmallKnob.svg")));
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

struct PinwheelRust : app::SvgSwitch {
	PinwheelRust() {
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/PinwheelRust_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/PinwheelRust_1.svg")));
	}
};

struct ChowLabel : ui::Label {
    std::string fontPath;

    void draw(const DrawArgs& args) override {
        std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
        nvgFontSize(args.vg, fontSize);
        nvgFontFaceId(args.vg, font->handle);
        nvgTextLetterSpacing(args.vg, 0.2f);
        Vec textPos = Vec(0.0f, 0.0f);

        int align = NVG_ALIGN_CENTER;
        switch(alignment) {
            case Label::LEFT_ALIGNMENT: align = NVG_ALIGN_LEFT; break;
            case Label::CENTER_ALIGNMENT: align = NVG_ALIGN_CENTER; break;
            case Label::RIGHT_ALIGNMENT: align = NVG_ALIGN_RIGHT; break;
        }

        nvgFillColor(args.vg, color);
        nvgTextAlign(args.vg, align);
        nvgText(args.vg, textPos.x, textPos.y, text.c_str(), nullptr);
    }
};
