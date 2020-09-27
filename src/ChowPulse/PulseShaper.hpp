#pragma once

#include <wdf/wdf.h>

class PulseShaper
{
public:
    PulseShaper() = default;

    void reset (double sampleRate)
    {
        c40 = std::make_unique<WDF::Capacitor> (0.015e-6, sampleRate, 0.029);
        P1 = std::make_unique<WDF::WDFParallel> (c40.get(), &r163);
        S1 = std::make_unique<WDF::WDFSeries> (&Vs, P1.get());

        I1 = std::make_unique<WDF::PolarityInverter> (&r162);
        P2 = std::make_unique<WDF::WDFParallel> (I1.get(), S1.get());

        d53.connectToNode (P2.get());
    }

    void setResistors (float r162_ohms, float r163_ohms)
    {
        r162.setResistanceValue ((double) r162_ohms);
        r163.setResistanceValue ((double) r163_ohms);
    }

    inline float processSample (float x) noexcept
    {
        Vs.setVoltage ((double) x);

        d53.incident (P2->reflected());
        double y = r162.voltage();
        P2->incident (d53.reflected());

        return (float) y;
    }

private:
    WDF::ResistiveVoltageSource Vs;

    WDF::Resistor r162 {   4700.0 };
    WDF::Resistor r163 { 100000.0 };
    std::unique_ptr<WDF::Capacitor> c40;
    WDF::Diode d53 { 2.52e-9, 25.85e-3 }; // 1N4148 diode
    
    std::unique_ptr<WDF::PolarityInverter> I1;
    std::unique_ptr<WDF::WDFSeries> S1;
    std::unique_ptr<WDF::WDFParallel> P1;
    std::unique_ptr<WDF::WDFParallel> P2;
};
