#ifndef LAYERJSON_H_INCLUDED
#define LAYERJSON_H_INCLUDED

#include <rack.hpp>
#include <MLUtils/Model.h>

// utility functions to lave RNN layer weights to JSON
class LayerJson {
public:
    static json_t* DenseToJson(MLUtils::Dense<float>* dense);
    static json_t* GruToJson(MLUtils::GRULayer<float>* gru);

    static void JsonToDense(MLUtils::Dense<float>* dense, json_t* json);
    static void JsonToGru(MLUtils::GRULayer<float>* gru, json_t* json);

private:
    LayerJson() = default; // static class
};

#endif // LAYERJSON_H_INCLUDED
