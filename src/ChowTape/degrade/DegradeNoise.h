#pragma once

#include "../../shared/SmoothedValue.h"
#include <rack.hpp>

class DegradeNoise
{
public:
    DegradeNoise() = default;
    ~DegradeNoise() = default;

    void setGain (float newGain)
    {
        gain.setTargetValue(newGain);
    }

    void prepare (float sampleRate)
    {
        gain.reset((double) sampleRate, 0.05);
    }

    inline float processSample(float x)
    {
        return x + (rack::random::uniform() - 0.5f) * gain.getNextValue();
    }

private:
    SmoothedValue<float, ValueSmoothingTypes::Linear> gain;
};
