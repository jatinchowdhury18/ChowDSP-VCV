#pragma once
#include <rack.hpp>
#include "shared/components.hpp"

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
extern Model* modelChowTape;
extern Model* modelChowPhaserFeedback;
extern Model* modelChowPhaserMod;
extern Model* modelChowFDN;
extern Model* modelChowRNN;
extern Model* modelChowModal;
extern Model* modelChowDer;

void createScrews(ModuleWidget& mw);
