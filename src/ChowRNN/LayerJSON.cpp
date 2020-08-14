#include "LayerJSON.hpp"

// serialize a 1D array as accessed buy func(i)
json_t* serialize_array(size_t nDims, std::function<json_t*(size_t)> func) {
    json_t* array = json_array();

    for(size_t i = 0; i < nDims; ++i)
        json_array_insert_new(array, i, func(i));

    return array;
}

// deserialize 1D array into float vector
std::vector<float> deserialize_array(json_t* array, size_t nDims) {
    std::vector<float> vec (nDims, 0.0f);

    if(array) {
        for(size_t i = 0; i < nDims; ++i)
            vec[i] = (float) json_real_value(json_array_get(array, i));
    }

    return vec;
}

// serialize a 2D array as accessed buy func(i, k)
json_t* serialize_double_array(size_t nDims1, size_t nDims2, std::function<json_t*(size_t,size_t)> func) {
    json_t* array = json_array();

    for(size_t i = 0; i < nDims1; ++i) {
        auto subArray = serialize_array(nDims2, [=] (size_t k) { return func(i, k); });
        json_array_insert_new(array, i, subArray);
    }

    return array;
}

// deserialize 2D array into 2D float vector
std::vector<std::vector<float>> deserialize_double_array(json_t* array, size_t nDims1, size_t nDims2) {
    std::vector<std::vector<float>> vec;

    if(array) {
        for(size_t i = 0; i < nDims1; ++i) {
            auto subArray = json_array_get(array, i);
            auto subVec = deserialize_array(subArray, nDims2);
            vec.push_back(subVec);
        }
    }

    return vec;
}

json_t* LayerJson::DenseToJson(MLUtils::Dense<float>* dense) {
    json_t* rootJ = json_object();

    // serialize weights
    json_t* weightsJ = serialize_double_array(dense->out_size, dense->in_size,
        [=] (size_t i, size_t k) { return json_real((double) dense->getWeight(i, k)); });
    json_object_set_new(rootJ, "weights", weightsJ);

    // serialize biases
    json_t* biasJ = serialize_array(dense->out_size, [=] (size_t i) { return json_real((double) dense->getBias(i)); });
    json_object_set_new(rootJ, "bias", biasJ);

    return rootJ;
}

json_t* LayerJson::GruToJson(MLUtils::GRULayer<float>* gru) {
    json_t* rootJ = json_object();

    // serialize kernel weights
    json_t* kernelWeightsJ = serialize_double_array(gru->in_size, 3 * gru->out_size,
        [=] (size_t i, size_t k) { return json_real((double) gru->getWVal(i, k)); });
    json_object_set_new(rootJ, "kernelWeights", kernelWeightsJ);

    // serialize recurrent weights
    json_t* recurrentWeightsJ = serialize_double_array(gru->out_size, 3 * gru->out_size,
        [=] (size_t i, size_t k) { return json_real((double) gru->getUVal(i, k)); });
    json_object_set_new(rootJ, "recurrentWeights", recurrentWeightsJ);

    // serialize biases
    json_t* biasJ = serialize_double_array(2, 3 * gru->out_size,
        [=] (size_t i, size_t k) { return json_real((double) gru->getBVal(i, k)); });
    json_object_set_new(rootJ, "bias", biasJ);

    return rootJ;
}

void LayerJson::JsonToDense(MLUtils::Dense<float>* dense, json_t* json)
{
    // de-serialize weights
    json_t* weightsJ = json_object_get(json, "weights");
    if(weightsJ) {
        auto denseWeights = deserialize_double_array(weightsJ, dense->out_size, dense->in_size);
        auto weightsPtr = denseWeights[0].data();
        dense->setWeights(&weightsPtr);
    }

    // de-serialize bias
    json_t* biasJ = json_object_get(json, "bias");
    if(biasJ) {
        auto bias = deserialize_array(biasJ, dense->out_size);
        dense->setBias(bias.data());
    }
}

void LayerJson::JsonToGru(MLUtils::GRULayer<float>* gru, json_t* json)
{
    // de-serialize kernel weights
    json_t* kernelWeightsJ = json_object_get(json, "kernelWeights");
    if(kernelWeightsJ) {
        auto kernelWeights = deserialize_double_array(kernelWeightsJ, gru->in_size, 3 * gru->out_size);
        auto weightsPtr = kernelWeights[0].data();
        gru->setWVals(&weightsPtr);
    }

    // de-serialize recurrent weights
    json_t* recurrentWeightsJ = json_object_get(json, "recurrentWeights");
    if(recurrentWeightsJ) {
        auto recurrentWeights = deserialize_double_array(recurrentWeightsJ, gru->out_size, 3 * gru->out_size);
        auto weightsPtr = recurrentWeights[0].data();
        gru->setUVals(&weightsPtr);
    }

    // de-serialize biases
    json_t* biasJ = json_object_get(json, "bias");
    if(biasJ) {
        auto bias = deserialize_double_array(biasJ, 2, 3 * gru->out_size);
        auto weightsPtr = bias[0].data();
        gru->setBVals(&weightsPtr);
    }
}
