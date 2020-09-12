#pragma once

#include "iir.hpp"

/**
 * Nonlinear biquad filter, using the "Nonlinear Feedback"
 * structure developed in
 * https://dafx2020.mdw.ac.at/proceedings/papers/DAFx2020_paper_3.pdf
 */ 
struct NLBiquad : public BiquadFilter {
    float drive = 1.0f;

    void setDrive(float newDrive) { drive = newDrive; }

    inline float process(float x) {
        float y = z[1] + x * b[0];
        auto yDrive = driveFunc(y, drive);
        z[1] = z[2] + x * b[1] - yDrive * a[1];
        z[2] = x * b[2] - yDrive * a[2];
        return y;
    }

    inline float driveFunc(float x, float d) {
        return std::tanh (x * d) / d;
    }
};
