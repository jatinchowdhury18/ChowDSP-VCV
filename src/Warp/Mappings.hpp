#pragma once

#include "Warp.hpp"

namespace WarpMappings {

constexpr size_t NumMappings = 3;

/**
 * Parameter mapping #1, by Jatin Chowdhury
 * ---
 * Cutoff -> Freq
 * Heat -> Q, Drive (inverted)
 * Width -> Gain, FB
 * Drive -> FB Drive
 */
ParamMap::ParamMapSet mapping1(Warp& warp, WarpFilter& warpFilter) {
    return {
        { warp, warpFilter, Warp::CUTOFF_PARAM, { WarpFilter::FREQ_PARAM } },
        { warp, warpFilter, Warp::HEAT_PARAM, { WarpFilter::Q_PARAM }, 0.85f, 0.15f },
        { warp, warpFilter, Warp::HEAT_PARAM, { WarpFilter::DRIVE_PARAM }, -9.0f, 10.0f },
        { warp, warpFilter, Warp::WIDTH_PARAM, { WarpFilter::GAIN_PARAM }, 48.0f, -24.0f },
        { warp, warpFilter, Warp::WIDTH_PARAM, { WarpFilter::FB_PARAM }, 0.9f },
        { warp, warpFilter, Warp::DRIVE_PARAM, { WarpFilter::FB_DRIVE_PARAM }, 9.0f, 1.0f },
    };
}

/**
 * Parameter mapping #2, by Isabel Kaspriskie
 * ---
 * Cutoff -> Freq
 * Heat -> Drive, FB
 * Width -> Gain (~half scale), Q (inverted)
 * Drive -> FB Drive
 */
ParamMap::ParamMapSet mapping2(Warp& warp, WarpFilter& warpFilter) {
    return {
        { warp, warpFilter, Warp::CUTOFF_PARAM, { WarpFilter::FREQ_PARAM } },
        { warp, warpFilter, Warp::HEAT_PARAM, { WarpFilter::DRIVE_PARAM }, 9.0f, 1.0f },
        { warp, warpFilter, Warp::HEAT_PARAM, { WarpFilter::FB_PARAM }, 0.9f },
        { warp, warpFilter, Warp::WIDTH_PARAM, { WarpFilter::GAIN_PARAM }, 28.0f, -14.0f },
        { warp, warpFilter, Warp::WIDTH_PARAM, { WarpFilter::Q_PARAM }, -0.85f, 1.0f },
        { warp, warpFilter, Warp::DRIVE_PARAM, { WarpFilter::FB_DRIVE_PARAM }, 9.0f, 1.0f },
    };
}

/**
 * Parameter mapping #3, by Jatin Chowdhury
 * ---
 * Cutoff -> Freq
 * Heat -> FB Drive, FB
 * Width -> Q (inverted)
 * Drive -> Drive, Gain (0 - 24 dB)
 */
ParamMap::ParamMapSet mapping3(Warp& warp, WarpFilter& warpFilter) {
    return {
        { warp, warpFilter, Warp::CUTOFF_PARAM, { WarpFilter::FREQ_PARAM } },
        { warp, warpFilter, Warp::HEAT_PARAM, { WarpFilter::FB_DRIVE_PARAM }, 9.0f, 1.0f },
        { warp, warpFilter, Warp::HEAT_PARAM, { WarpFilter::FB_PARAM }, 0.9f },
        { warp, warpFilter, Warp::WIDTH_PARAM, { WarpFilter::Q_PARAM }, -0.85f, 1.0f },
        { warp, warpFilter, Warp::DRIVE_PARAM, { WarpFilter::DRIVE_PARAM }, 9.0f, 1.0f },
        { warp, warpFilter, Warp::DRIVE_PARAM, { WarpFilter::GAIN_PARAM }, 24.0f, 0.0f },
    };
}

} // WarpMappings
