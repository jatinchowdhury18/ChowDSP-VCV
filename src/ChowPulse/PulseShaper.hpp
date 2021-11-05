#pragma once

#include <wdf_t.h>

/**
 * Wave digital filter model of the pulse shaper
 * from the TR-808 kick drum circuit. For more information,
 * see: https://searchworks.stanford.edu/view/11891203
 */ 
class PulseShaper
{
public:
    PulseShaper (float sampleRate) : c40 (0.015e-6f, sampleRate, 0.029f)
    {
    }

    void setResistors (float r162_ohms, float r163_ohms)
    {
        r162.setResistanceValue (r162_ohms);
        r163.setResistanceValue (r163_ohms);
    }

    inline float processSample (float x) noexcept
    {
        Vs.setVoltage (x);

        d53.incident (P2.reflected());
        float y = chowdsp::WDFT::voltage<float> (r162);
        P2.incident (d53.reflected());

        return y;
    }

private:
    chowdsp::WDFT::CapacitorAlphaT<float> c40;
    chowdsp::WDFT::ResistorT<float> r163 { 100000.0f };
    chowdsp::WDFT::WDFParallelT<float, decltype(c40), decltype(r163)> P1 { c40, r163 };
    
    chowdsp::WDFT::ResistiveVoltageSourceT<float> Vs;
    chowdsp::WDFT::WDFSeriesT<float, decltype(Vs), decltype(P1)> S1 { Vs, P1 };

    chowdsp::WDFT::ResistorT<float> r162 { 4700.0f };
    chowdsp::WDFT::PolarityInverterT<float, decltype(r162)> I1 { r162 };

    chowdsp::WDFT::WDFParallelT<float, decltype(I1), decltype(S1)> P2 { I1, S1 };
    chowdsp::WDFT::DiodeT<float, decltype(P2)> d53 { P2, 2.52e-9f }; // 1N4148 diode
};
