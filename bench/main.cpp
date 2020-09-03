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
    runModuleBench(modelChowPhaserMod);

    ParamVec fdnParams { { 4, 2.0f } };
    runModuleBench(modelChowFDN, fdnParams);
    runModuleBench(modelChowRNN);
    runModuleBench(modelChowModal);
    runModuleBench(modelChowDer);

    return 0;
}
