#ifndef OVERSAMPLING_H_INCLUDED
#define OVERSAMPLING_H_INCLUDED

#include <plugin.hpp>
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

    void reset(float sampleRate) {
        float fc = 0.98f * (sampleRate / 2.0f);
        auto Qs = calculateButterQs(2*N);

        for(int i = 0; i < N; ++i)
            filters[i].setParameters(dsp::BiquadFilter::Type::LOWPASS, fc / sampleRate, Qs[i], 1.0f);
    }
    
    inline float process(float x) noexcept {
        for(int i = 0; i < N; ++i)
            x = filters[i].process(x);
        
        return x;
    }

private:
    dsp::BiquadFilter filters[N];
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
        aaFilter.reset(baseSampleRate);
        aiFilter.reset(baseSampleRate);
    }

    using OSProcess = std::function<float(float)>;
    OSProcess osProcess = [] (float x) { return x; };

    inline float process(float x) noexcept {
        float y = 0.0f;
		for (int k = 0; k < ratio; k++) {
			// upsample (insert zeros) and apply upsampling gain
            float upSample = (k == ratio - 1) ? ratio * x : 0.0f;
            
			// apply antiimaging filter
			upSample = aiFilter.process(upSample);
            
			// apply processing
            float procSample = osProcess(upSample);

			// apply antialiasing filter
			y = aaFilter.process(procSample);
		}

        return y;
    }

private:
    AAFilter<filtN> aaFilter; // anti-aliasing filter
    AAFilter<filtN> aiFilter; // anti-imaging filter
};

#endif // OVERSAMPLING_H_INCLUDED
