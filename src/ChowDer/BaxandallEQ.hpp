#pragma once

#include <memory>
#include <wdf_t.h>

/** Wave digital filters to eumlate the Baxandall EQ circuit. */

class BaxandallTreble;
class BaxandallBass;
class BaxandallEQ {
public:
    BaxandallEQ() = default;
    ~BaxandallEQ();

    void reset(float sampleRate);
    void setTreble(float trebleParam);
    void setBass(float bassParam);

    float processSample(float x) noexcept;

private:
    std::unique_ptr<BaxandallTreble> treble;
    std::unique_ptr<BaxandallBass> bass;

    float oldTrebleParam = 0.0f;
    float oldBassParam = 0.0f;
};
