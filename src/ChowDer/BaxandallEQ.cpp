#include "BaxandallEQ.hpp"
#include <rack.hpp>

using namespace chowdsp::WDFT;

namespace {
    constexpr float bassTaper = 5.644f;
    constexpr float trebleTaper = 3.889f;
    constexpr float minFloat = (float) 1.0e-6;
    constexpr float maxFloat = 1.0f - (float) 1.0e-6;
}

float resistorTaper(float val01, float alpha) {
    return std::pow(rack::clamp(val01, minFloat, maxFloat), alpha);
}

//////////////////////////////////
class BaxandallTreble {
public:
    BaxandallTreble (float sampleRate) : C4(22.0e-9f, sampleRate),
                                         C3(2.2e-9f, sampleRate)
    {
    }

    void setTreble(float treble) {
        treble = resistorTaper(treble, trebleTaper);
        VR2a.setResistanceValue(100000.0f * (1.0f - treble));
        VR2b.setResistanceValue(100000.0f * treble);
    }

    inline float processSample(float x) noexcept {
        Vin.setVoltage(x);

        Vin.incident(S3.reflected());
        S3.incident(Vin.reflected());
        auto y = voltage<float>(C4) + voltage<float>(VR2b);

        return y;
    }

private:
    ResistorT<float> VR2b { 100000.0f };
    CapacitorT<float> C4;
    WDFSeriesT<float, decltype(VR2b), decltype(C4)> S1 { VR2b, C4 };

    ResistorT<float> VR2a { 100000.0f };
    WDFSeriesT<float, decltype(S1), decltype(VR2a)> S2 { S1, VR2a };

    CapacitorT<float> C3;
    WDFSeriesT<float, decltype(C3), decltype(S2)> S3 { C3, S2 };

    IdealVoltageSourceT<float, decltype(S3)> Vin { S3 };
};

//////////////////////////////////
class BaxandallBass {
public:
    BaxandallBass (float sampleRate) : C1(22.0e-9f, sampleRate),
                                       C2(220.0e-9f, sampleRate),
                                       C4(22.0e-9f, sampleRate)
    {
    }

    void setTreble(float treble) {
        treble = resistorTaper(treble, trebleTaper);
        VR2b.setResistanceValue(100000.0f * treble);
    }
    void setBass(float bass) {
        bass = resistorTaper(bass, bassTaper);
        VR1b.setResistanceValue(100000.0f * (1.0f - bass));
        VR1a.setResistanceValue(100000.0f * bass);
    }

    inline float processSample(float x) noexcept {
        Vin.setVoltage(x);

        Vin.incident(S5.reflected());
        S5.incident(Vin.reflected());
        auto y = voltage<float>(C4) + voltage<float>(VR2b);

        return y;
    }

private:
    CapacitorT<float> C1;
    ResistorT<float> VR1a { 100000.0f };
    WDFParallelT<float, decltype(C1), decltype(VR1a)> P1 { C1, VR1a };
    
    CapacitorT<float> C2;
    ResistorT<float> VR1b { 100000.0f };
    WDFParallelT<float, decltype(C2), decltype(VR1b)> P2 { C2, VR1b };

    CapacitorT<float> C4;
    ResistorT<float> VR2b { 100000.0f };
    WDFSeriesT<float, decltype(C4), decltype(VR2b)> S1 { C4, VR2b };

    ResistorT<float> R3 { 10000.0f };
    WDFSeriesT<float, decltype(S1), decltype(R3)> S2 { S1, R3 };

    ResistorT<float> R2 { 1000.0f };
    WDFSeriesT<float, decltype(P1), decltype(R2)> S3 { P1, R2 };

    WDFParallelT<float, decltype(S3), decltype(S2)> P3 { S3, S2 };
    WDFSeriesT<float, decltype(P3), decltype(P2)> S4 { P3, P2 };

    ResistorT<float> R1 { 10000.0f };
    WDFSeriesT<float, decltype(S4), decltype(R1)> S5 { S4, R1 };

    IdealVoltageSourceT<float, decltype(S5)> Vin { S5 };
};

//////////////////////////////////
BaxandallEQ::~BaxandallEQ() = default;

void BaxandallEQ::reset(float sampleRate) {
    treble.reset(new BaxandallTreble(sampleRate));
    bass.reset(new BaxandallBass(sampleRate));
}

void BaxandallEQ::setTreble(float trebleParam) {
    if(oldTrebleParam == trebleParam) // no update needed
        return;

    oldTrebleParam = trebleParam;
    treble->setTreble(trebleParam);
    bass->setTreble(trebleParam);
}

void BaxandallEQ::setBass(float bassParam) {
    if(oldBassParam == bassParam)  // no update needed
        return;

    oldBassParam = bassParam;
    bass->setBass(bassParam);
}

float BaxandallEQ::processSample(float x) noexcept {
    return bass->processSample(x) + treble->processSample(x);
}
