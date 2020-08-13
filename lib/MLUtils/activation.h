#ifndef ACTIVATION_H_INCLUDED
#define ACTIVATION_H_INCLUDED

#include <functional>
#include "Layer.h"

namespace MLUtils
{

template<typename T>
class Activation : public Layer<T>
{
public:
    Activation (size_t size, std::function<T(T)> func) :
        Layer<T> (size, size),
        func (func)
    {}

    virtual ~Activation() {}

    inline void forward (const T* input, T* out) override
    {
        for (size_t i = 0; i < Layer<T>::out_size; ++i)
        {
            out[i] = func (input[i]);
        }
    }

private:
    const std::function<T(T)> func;
};

} // namespace MLUtils

#ifdef USE_EIGEN
#include <Eigen/Dense>

namespace MLUtils
{

template<typename T>
class TanhActivation : public Activation<T>
{
public:
    TanhActivation (size_t size) :
        Activation<T> (size, {})
    {
        vector.resize (size, 1);
    }

    inline void forward (const T* input, T* out) override
    {
        std::copy (input, &input[Layer<T>::in_size], out);
        vector = Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic, 1>> (out, Layer<T>::in_size, 1);
        vector = vector.array().tanh();
    }

    Eigen::Matrix<T, Eigen::Dynamic, 1> vector;
};

} // namespace MLUtils

#else
#include <cmath>

namespace MLUtils
{

template<typename T>
class TanhActivation : public Activation<T>
{
public:
    TanhActivation (size_t size) :
        Activation<T> (size, [] (float x) { return std::tanh (x); })
    {}
};

} // namespace MLUtils

#endif // USE_EIGEN

#endif // ACTIVATION_H_INCLUDED
