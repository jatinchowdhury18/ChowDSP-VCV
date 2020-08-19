/*
  ==============================================================================
   This file was copied (with many modifications) from the JUCE libaray.
   This copying is allowed under the terms of the GPL v3.
  ==============================================================================
*/

#ifndef DELAYLINE_H_INCLUDED
#define DELAYLINE_H_INCLUDED

#include "../plugin.hpp"
#include <type_traits>

//==============================================================================
/**
    A collection of structs to pass as the template argument when setting the
    interpolation type for the DelayLine class.
*/
namespace DelayLineInterpolationTypes
{
    /**
        No interpolation between successive samples in the delay line will be
        performed. This is useful when the delay is a constant integer or to
        create lo-fi audio effects.
    */
    struct None {};

    /**
        Successive samples in the delay line will be linearly interpolated. This
        type of interpolation has a low compuational cost where the delay can be
        modulated in real time, but it also introduces a low-pass filtering effect
        into your audio signal.
    */
    struct Linear {};

    /**
        Successive samples in the delay line will be interpolated using a 3rd order
        Lagrange interpolator. This method incurs more computational overhead than
        linear interpolation but reduces the low-pass filtering effect whilst
        remaining amenable to real time delay modulation.
    */
    struct Lagrange3rd {};

    /**
        Successive samples in the delay line will be interpolated using 1st order
        Thiran interpolation. This method is very efficient, and features a flat
        amplitude frequency response in exchange for less accuracy in the phase
        response. This interpolation method is stateful so is unsuitable for
        applications requiring fast delay modulation.
    */
    struct Thiran {};
}

//==============================================================================
/**
    A delay line processor featuring several algorithms for the fractional delay
    calculation, block processing, and sample-by-sample processing useful when
    modulating the delay in real time or creating a standard delay effect with
    feedback.

    Note: If you intend to change the delay in real time, you may want to smooth
    changes to the delay systematically using either a ramp or a low-pass filter.
*/
template <typename SampleType, typename InterpolationType = DelayLineInterpolationTypes::Linear>
class DelayLine
{
public:
    //==============================================================================
    /** Default constructor. */
    DelayLine();

    //==============================================================================
    /** Sets the delay in samples. */
    void setDelay (SampleType newDelayInSamples);

    /** Returns the current delay in samples. */
    SampleType getDelay() const;

    /** Resets the internal state variables of the processor. */
    void reset();

    //==============================================================================
    /** Pushes a single sample into one channel of the delay line. */
    inline void pushSample (SampleType sample) noexcept
    {
        bufferData[writePos] = sample;
        writePos = (writePos + HISTORY_SIZE - 1) % HISTORY_SIZE;
    }

    /** Pops a single sample from one channel of the delay line. */
    inline SampleType popSample() noexcept
    {
        return interpolateSample();
    }

    inline void updateReadPointer() noexcept
    {
        readPos = (readPos + HISTORY_SIZE - 1) % HISTORY_SIZE;
    }

    inline SampleType process (SampleType x) noexcept
    {
        pushSample (x);
        auto y = popSample();
        updateReadPointer();

        return y;
    }

private:
    //==============================================================================
    template <typename T = InterpolationType>
    typename std::enable_if <std::is_same <T, DelayLineInterpolationTypes::None>::value, SampleType>::type
    interpolateSample() const
    {
        auto index = (readPos + delayInt) % HISTORY_SIZE;
        return bufferData[index];
    }

    template <typename T = InterpolationType>
    typename std::enable_if <std::is_same <T, DelayLineInterpolationTypes::Linear>::value, SampleType>::type
    interpolateSample() const
    {
        auto index1 = readPos + delayInt;
        auto index2 = index1 + 1;

        if (index2 >= HISTORY_SIZE)
        {
            index1 %= HISTORY_SIZE;
            index2 %= HISTORY_SIZE;
        }

        auto value1 = bufferData[index1];
        auto value2 = bufferData[index2];

        return value1 + delayFrac * (value2 - value1);
    }

    template <typename T = InterpolationType>
    typename std::enable_if <std::is_same <T, DelayLineInterpolationTypes::Lagrange3rd>::value, SampleType>::type
    interpolateSample() const
    {
        auto index1 = readPos + delayInt;
        auto index2 = index1 + 1;
        auto index3 = index2 + 1;
        auto index4 = index3 + 1;

        if (index4 >= HISTORY_SIZE)
        {
            index1 %= HISTORY_SIZE;
            index2 %= HISTORY_SIZE;
            index3 %= HISTORY_SIZE;
            index4 %= HISTORY_SIZE;
        }

        auto value1 = bufferData[index1];
        auto value2 = bufferData[index2];
        auto value3 = bufferData[index3];
        auto value4 = bufferData[index4];

        auto d1 = delayFrac - 1.f;
        auto d2 = delayFrac - 2.f;
        auto d3 = delayFrac - 3.f;

        auto c1 = -d1 * d2 * d3 / 6.f;
        auto c2 = d2 * d3 * 0.5f;
        auto c3 = -d1 * d3 * 0.5f;
        auto c4 = d1 * d2 / 6.f;

        return value1 * c1 + delayFrac * (value2 * c2 + value3 * c3 + value4 * c4);
    }

    template <typename T = InterpolationType>
    typename std::enable_if <std::is_same <T, DelayLineInterpolationTypes::Thiran>::value, SampleType>::type
    interpolateSample()
    {
        auto index1 = readPos + delayInt;
        auto index2 = index1 + 1;

        if (index2 >= HISTORY_SIZE)
        {
            index1 %= HISTORY_SIZE;
            index2 %= HISTORY_SIZE;
        }

        auto value1 = bufferData[index1];
        auto value2 = bufferData[index2];

        auto output = delayFrac == 0 ? value1 : value2 + alpha * (value1 - v);
        v = output;

        return output;
    }

    //==============================================================================
    template <typename T = InterpolationType>
    typename std::enable_if <std::is_same <T, DelayLineInterpolationTypes::None>::value, void>::type
    updateInternalVariables()
    {
    }

    template <typename T = InterpolationType>
    typename std::enable_if <std::is_same <T, DelayLineInterpolationTypes::Linear>::value, void>::type
    updateInternalVariables()
    {
    }

    template <typename T = InterpolationType>
    typename std::enable_if <std::is_same <T, DelayLineInterpolationTypes::Lagrange3rd>::value, void>::type
    updateInternalVariables()
    {
        if (delayInt >= 1)
        {
            delayFrac++;
            delayInt--;
        }
    }

    template <typename T = InterpolationType>
    typename std::enable_if <std::is_same <T, DelayLineInterpolationTypes::Thiran>::value, void>::type
    updateInternalVariables()
    {
        if (delayFrac < (SampleType) 0.618 && delayInt >= 1)
        {
            delayFrac++;
            delayInt--;
        }

        alpha = (1 - delayFrac) / (1 + delayFrac);
    }

    //==============================================================================
    enum {
        HISTORY_SIZE = 1 << 21,
    };

    double sampleRate;
    SampleType bufferData[HISTORY_SIZE];
    
    SampleType v;
    int writePos, readPos;
    SampleType delay = 0.0, delayFrac = 0.0;
    int delayInt = 0;
    SampleType alpha = 0.0;
};

#endif // DELAYLINE_H_INCLUDED
