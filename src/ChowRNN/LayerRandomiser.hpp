#ifndef LAYERRANDOMISER_H_INCLUDED
#define LAYERRANDOMISER_H_INCLUDED

#include <MLUtils/Model.h>
#include <random>

// utility functions to randomise the weights of an RNN layer
class LayerRandomiser {
public:
    LayerRandomiser() = default;

    void randomDenseWeights(MLUtils::Dense<float>* dense);
    void randomDenseBias(MLUtils::Dense<float>* dense);
    void zeroDenseBias(MLUtils::Dense<float>* dense);

    void randomGRU(MLUtils::GRULayer<float>* gru);
    void randomKernelWeights(MLUtils::GRULayer<float>* gru);
    void randomRecurrentWeights(MLUtils::GRULayer<float>* gru);
    void randomGRUBias(MLUtils::GRULayer<float>* gru);

private:
    std::default_random_engine engine;
    std::uniform_real_distribution<float> denseWeightDist { -1.0f, 1.0f };
    std::uniform_real_distribution<float> biasDist { -0.5f, 0.5f };
    std::uniform_real_distribution<float> kernelDist { -1.0f, 1.0f };
    std::uniform_real_distribution<float> recurrentDist { -0.5f, 0.5f };
};

#endif // LAYERRANDOMISER_H_INCLUDED
