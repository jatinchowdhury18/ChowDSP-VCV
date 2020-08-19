#pragma once

#include <rack.hpp>
#include <memory>
#include "benchmark.hpp"

using ParamPair = std::pair<int, float>;
using ParamVec = std::vector<ParamPair>;

class ModuleBench : public Benchmark {
public:
    ModuleBench (rack::plugin::Model* model, ParamVec params = {});
    ~ModuleBench();

    void setParams (ParamVec params);
    void generateBuffers();
    double getNumSeconds() const noexcept;
    void process();

private:
    std::unique_ptr<rack::engine::Module> module;
    rack::engine::Module::ProcessArgs args;

    std::vector<std::vector<float>> inputs;
};
