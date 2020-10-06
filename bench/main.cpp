#include <iostream>
#include "../src/plugin.hpp"
#include "modulebench.hpp"

void runModuleBench(rack::plugin::Model* model, ParamVec params = {}) {
    std::cout << "Running " << model->slug << " bench..." << std::endl;

    ModuleBench bench (model, params);

    auto time = bench.run();
    auto audioTime = bench.getNumSeconds();
    std::cout << audioTime / time << "x real-time" << std::endl;
}

int main()
{
    runModuleBench(modelChowTape);
    runModuleBench(modelChowPhaserFeedback);
    
    ParamVec phaserModParams { { 2, 50.0f } };
    runModuleBench(modelChowPhaserMod, phaserModParams);

    ParamVec fdnParams { { 4, 16.0f } };
    runModuleBench(modelChowFDN, fdnParams);

    runModuleBench(modelChowRNN);
    runModuleBench(modelChowModal);
    runModuleBench(modelChowDer);

    runModuleBench(modelWarp);
    runModuleBench(modelWerner);
    runModuleBench(modelChowPulse);

    return 0;
}
