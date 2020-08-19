#include <iostream>
#include "../src/plugin.hpp"
#include "modulebench.hpp"
#include "benchmark.hpp"

void runModuleBench(rack::plugin::Model* model, ParamVec params = {}) {
    std::cout << "Running " << model->slug << " bench..." << std::endl;
    ModuleBench bench (model);
    auto time = bench.run();
    auto audioTime = bench.getNumSeconds();
    std::cout << audioTime / time << "x real-time" << std::endl;
}

int main()
{
    runModuleBench(modelChowTape);
    runModuleBench(modelChowPhaserFeedback);
    runModuleBench(modelChowPhaserMod);

    ParamVec fdnParams { { 4, 16.0f } };
    runModuleBench(modelChowFDN, fdnParams);
    runModuleBench(modelChowRNN);
    // runModuleBench(modelChowModal);

    return 0;
}
