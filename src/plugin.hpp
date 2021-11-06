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
extern Model* modelWarp;
extern Model* modelWerner;
extern Model* modelCredit;
extern Model* modelChowPulse;
extern Model* modelChowTapeCompression;
extern Model* modelChowTapeChew;
extern Model* modelChowTapeDegrade;
extern Model* modelChowTapeLoss;
extern Model* modelChowChorus;

void createScrews(ModuleWidget& mw);
void addPubToMenu(Menu *menu, const std::string& pub);

inline float getSampleRate() noexcept {
// in benchmarking app, sample rate is not defined    
#ifndef CHOWDSP_BENCH
    return APP->engine->getSampleRate();
#else
    return 48000.0f;
#endif
}
