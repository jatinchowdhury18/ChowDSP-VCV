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

    void reset(float sampleRate);
    
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

    void reset(float baseSampleRate);

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
