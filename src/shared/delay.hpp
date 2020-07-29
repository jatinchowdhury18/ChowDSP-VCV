#ifndef DELAY_H_INCLUDED
#define DELAY_H_INCLUDED

#include "../plugin.hpp"
#include "samplerate.h"

class Delay {
public:
    Delay() {
        src = src_new(SRC_SINC_FASTEST, 1, NULL);
        assert(src);
    }

    ~Delay() {
        src_delete(src);
    }

    void setProcessArgs(const Module::ProcessArgs& args) {
        sampleRate = args.sampleRate;
    }

    void setDelayTimeMs(float timeMs) {
        delaySamples = std::round(sampleRate * timeMs / 1000.0f);
    }

    void reset() {
        histBuffer.clear();
        outBuffer.clear();
        
        src_reset(src);
    }

    inline float process(float x) {
        // return x;

        // push dry sample into history buffer
        if(! histBuffer.full()) {
            histBuffer.push(x);
        }

        // how many samples do we need to consume to catch up?
        float consume = delaySamples - histBuffer.size();

        if(outBuffer.empty()) {
            double ratio = 1.0;
            if(std::fabs(consume) >= 16.0f) {
                ratio = std::pow(10.0f, clamp(consume / 10000.0f, -1.0f, 1.0f));
            }

            SRC_DATA srcData;
            srcData.data_in = (const float*) histBuffer.startData();
            srcData.data_out = (float*) outBuffer.endData();
            srcData.input_frames = std::min((int) histBuffer.size(), 16);
            srcData.output_frames = outBuffer.capacity();
            srcData.end_of_input = false;
            srcData.src_ratio = ratio;

            src_process(src, &srcData);
            histBuffer.startIncr(srcData.input_frames_used);
            outBuffer.endIncr(srcData.output_frames_gen);
        }

        float y = 0.0f;
        if(! outBuffer.empty()) {
            y = outBuffer.shift();
        }

        return y;
    }

    inline void write(float x) { readSample = process(x); }
    inline float read() const noexcept { return readSample; }

private:
    enum {
        HISTORY_SIZE = 1 << 21,
    };

    dsp::DoubleRingBuffer<float, HISTORY_SIZE> histBuffer;
    dsp::DoubleRingBuffer<float, 16> outBuffer;

    SRC_STATE* src;

    float sampleRate = 44100.0f;
    float delaySamples = 0.0f;
    float readSample = 0.0f;
};

#endif // DELAY_H_INCLUDED
