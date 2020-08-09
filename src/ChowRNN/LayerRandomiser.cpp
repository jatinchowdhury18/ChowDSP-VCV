#include "LayerRandomiser.hpp"

void LayerRandomiser::randomDenseWeights(MLUtils::Dense<float>* dense) {
    std::vector<std::vector<float>> denseWeights;

    for(size_t i = 0; i < dense->out_size; ++i) {
        std::vector<float> weights(dense->in_size);

        for(size_t j = 0; j < dense->in_size; ++j)
            weights[j] = denseWeightDist(engine);

        denseWeights.push_back(weights);
    }

    auto weightsPtr = denseWeights[0].data();
    dense->setWeights(&weightsPtr);
}

void LayerRandomiser::randomDenseBias(MLUtils::Dense<float>* dense) {
    std::vector<float> bias (dense->out_size);

    for(size_t i = 0; i < dense->out_size; ++i)
        bias[i] = biasDist(engine);

    dense->setBias(bias.data());
}

void LayerRandomiser::zeroDenseBias(MLUtils::Dense<float>* dense) {
    std::vector<float> bias (dense->out_size);

    std::fill(bias.begin(), bias.end(), 0.0f);
    dense->setBias(bias.data());
}

void LayerRandomiser::randomGRU(MLUtils::GRULayer<float>* gru) {
    randomKernelWeights(gru);
    randomRecurrentWeights(gru);
    randomGRUBias(gru);
}

void LayerRandomiser::randomKernelWeights(MLUtils::GRULayer<float>* gru) {
    std::vector<std::vector<float>> kernelWeights;
    
    for(size_t i = 0; i < gru->in_size; ++i) {
        std::vector<float> weights (3 * gru->out_size);

        for(size_t j = 0; j < 3 * gru->out_size; ++j)
            weights[j] = kernelDist(engine);

        kernelWeights.push_back(weights);
    }

    auto weightsPtr = kernelWeights[0].data();
    gru->setWVals(&weightsPtr);
}

void LayerRandomiser::randomRecurrentWeights(MLUtils::GRULayer<float>* gru) {
    std::vector<std::vector<float>> recurrentWeights;

    for(size_t i = 0; i < gru->out_size; ++i) {
        std::vector<float> weights (3 * gru->out_size);

        for(size_t j = 0; j < 3 * gru->out_size; ++j)
            weights[j] = recurrentDist(engine);

        recurrentWeights.push_back(weights);
    }

    auto weightsPtr = recurrentWeights[0].data();
    gru->setUVals(&weightsPtr);
}

void LayerRandomiser::randomGRUBias(MLUtils::GRULayer<float>* gru) {
    std::vector<std::vector<float>> gruBias;

    for(size_t i = 0; i < 2; ++i) {
        std::vector<float> weights(3 * gru->out_size);

        for(size_t j = 0; j < 3 * gru->out_size; ++j)
            weights[j] = biasDist(engine);

        gruBias.push_back(weights);
    }

    auto weightsPtr = gruBias[0].data();
    gru->setBVals(&weightsPtr);
}
