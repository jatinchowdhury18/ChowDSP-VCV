#include "delay_line.hpp"

template <typename SampleType, typename InterpolationType>
DelayLine<SampleType, InterpolationType>::DelayLine()
{
    sampleRate = 44100.0;
}

//==============================================================================
template <typename SampleType, typename InterpolationType>
void DelayLine<SampleType, InterpolationType>::setDelay (SampleType newDelayInSamples)
{
    auto upperLimit = (SampleType) (HISTORY_SIZE - 1);
    
    delay     = clamp (newDelayInSamples, (SampleType) 0, upperLimit);
    delayInt  = static_cast<int> (std::floor (delay));
    delayFrac = delay - (SampleType) delayInt;

    updateInternalVariables();
}

template <typename SampleType, typename InterpolationType>
SampleType DelayLine<SampleType, InterpolationType>::getDelay() const
{
    return delay;
}

//==============================================================================
template <typename SampleType, typename InterpolationType>
void DelayLine<SampleType, InterpolationType>::reset()
{
    writePos = 0;
    readPos = 0;
    v = (SampleType) 0;

    std::fill (bufferData, &bufferData[HISTORY_SIZE], static_cast<SampleType> (0));
}

//==============================================================================
template class DelayLine<float,  DelayLineInterpolationTypes::None>;
template class DelayLine<float,  DelayLineInterpolationTypes::Linear>;
template class DelayLine<float,  DelayLineInterpolationTypes::Lagrange3rd>;
template class DelayLine<float,  DelayLineInterpolationTypes::Thiran>;
