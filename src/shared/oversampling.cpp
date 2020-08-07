#include "oversampling.hpp"

/** Calculate Q values for a Butterworth filter of a given order */
std::vector<float> calculateButterQs(int order) {
    const int lim = int (order / 2);
    std::vector<float> Qs;

    for(int k = 1; k <= lim; ++k) {
        auto b = -2.0f * std::cos((2.0f * k + order - 1) * 3.14159 / (2.0f * order));
        Qs.push_back(1.0f / b);
    }

    std::reverse(Qs.begin(), Qs.end());
    return Qs;
}

template<int N>
void AAFilter<N>::reset(float sampleRate) {
    float fc = 0.98f * (sampleRate / 2.0f);
    auto Qs = calculateButterQs(2*N);

    for(int i = 0; i < N; ++i)
        filters[i].setParameters(dsp::BiquadFilter::Type::LOWPASS, fc / sampleRate, Qs[i], 1.0f);
}

//===========================================
template class AAFilter<2>;
template class AAFilter<4>;
template class AAFilter<8>;

//====================================
template<int ratio, int filtN>
void OversampledProcess<ratio, filtN>::reset(float baseSampleRate) {
    aaFilter.reset(baseSampleRate);
    aiFilter.reset(baseSampleRate);
}

template class OversampledProcess<2>;
template class OversampledProcess<4>;
template class OversampledProcess<8>;
