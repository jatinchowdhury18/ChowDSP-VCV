#ifndef OVERSAMPLING_H_INCLUDED
#define OVERSAMPLING_H_INCLUDED

#include "iir.hpp"
#include <type_traits>
#include <functional>

/** 
    High-order filter to be used for anti-aliasing or anti-imaging.
    The template parameter N should be 1/2 the desired filter order.
*/
template<int N>
class AAFilter
{
public:
    AAFilter() = default;

    /** Calculate Q values for a Butterworth filter of a given order */
    static std::vector<float> calculateButterQs(int order) {
        const int lim = int (order / 2);
        std::vector<float> Qs;

        for(int k = 1; k <= lim; ++k) {
            auto b = -2.0f * std::cos((2.0f * k + order - 1) * 3.14159 / (2.0f * order));
            Qs.push_back(1.0f / b);
        }

        std::reverse(Qs.begin(), Qs.end());
        return Qs;
    }

    void reset(float sampleRate, int osRatio) {
        float fc = 0.98f * (sampleRate / 2.0f);
        auto Qs = calculateButterQs(2*N);

        for(int i = 0; i < N; ++i)
            filters[i].setParameters(BiquadFilter::Type::LOWPASS, fc / (osRatio * sampleRate), Qs[i], 1.0f);
    }
    
    inline float process(float x) noexcept {
        for(int i = 0; i < N; ++i)
            x = filters[i].process(x);
        
        return x;
    }

private:
    BiquadFilter filters[N];
};

/** 
    Class to implement an oversampled process.
*/
template<int ratio, int filtN = 4>
class OversampledProcess
{
public:
    OversampledProcess() = default;

    void reset(float baseSampleRate) {
        aaFilter.reset(baseSampleRate, ratio);
        aiFilter.reset(baseSampleRate, ratio);
        std::fill(osBuffer, &osBuffer[ratio], 0.0f);
    }

    inline void upsample(float x) noexcept {
        osBuffer[0] = x;
        std::fill(&osBuffer[1], &osBuffer[ratio], 0.0f);

        for(int k = 0; k < ratio; k++)
            osBuffer[k] = aiFilter.process(osBuffer[k]);
    }

    inline float downsample() noexcept {
        float y = 0.0f;
        for(int k = 0; k < ratio; k++)
            y = aaFilter.process(osBuffer[k]);

        return y;
    }

    float osBuffer[ratio];

private:
    AAFilter<filtN> aaFilter; // anti-aliasing filter
    AAFilter<filtN> aiFilter; // anti-imaging filter
};

#endif // OVERSAMPLING_H_INCLUDED
