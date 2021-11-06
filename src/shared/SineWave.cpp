#include "SineWave.h"

namespace chowdsp
{
template <typename T>
void SineWave<T>::setFrequency (T newFrequency) noexcept
{
    freq = newFrequency;
    eps = 2 * std::sin (M_PI * newFrequency / fs);
}

template <typename T>
void SineWave<T>::prepare (double sampleRate) noexcept
{
    fs = static_cast<T> (sampleRate);
    reset();
}

template <typename T>
void SineWave<T>::reset() noexcept
{
    // reset state to be "in phase"
    x1 = -1.0f;
    x2 = 0.0f;
}

template <typename T>
void SineWave<T>::reset (T phase) noexcept
{
    x1 = std::sin (phase);
    x2 = std::cos (phase);
}
} // namespace chowdsp

template class chowdsp::SineWave<float>;
template class chowdsp::SineWave<double>;
