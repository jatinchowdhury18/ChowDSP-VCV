#pragma once

#include "ChowWarp.hpp"

namespace ChowWarpMappings {

constexpr size_t NumMappings = 2;

/**
 * Parameter mapping #1, by Jatin Chowdhury
 * ---
 * Cutoff -> Freq
 * Heat -> Q, Drive (inverted)
 * Width -> Gain, FB
 * Drive -> FB Drive
 */
ParamMap::ParamMapSet mapping1(ChowWarp& chowWarp, WarpFilter& warpFilter) {
    return {
        { chowWarp, warpFilter, ChowWarp::CUTOFF_PARAM, { WarpFilter::FREQ_PARAM } },
        { chowWarp, warpFilter, ChowWarp::HEAT_PARAM, { WarpFilter::Q_PARAM }, 0.85f, 0.15f },
        { chowWarp, warpFilter, ChowWarp::HEAT_PARAM, { WarpFilter::DRIVE_PARAM }, -9.0f, 10.0f },
        { chowWarp, warpFilter, ChowWarp::WIDTH_PARAM, { WarpFilter::GAIN_PARAM }, 48.0f, -24.0f },
        { chowWarp, warpFilter, ChowWarp::WIDTH_PARAM, { WarpFilter::FB_PARAM }, 0.9f },
        { chowWarp, warpFilter, ChowWarp::DRIVE_PARAM, { WarpFilter::FB_DRIVE_PARAM }, 9.0f, 1.0f },
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
ParamMap::ParamMapSet mapping2(ChowWarp& chowWarp, WarpFilter& warpFilter) {
    return {
        { chowWarp, warpFilter, ChowWarp::CUTOFF_PARAM, { WarpFilter::FREQ_PARAM } },
        { chowWarp, warpFilter, ChowWarp::HEAT_PARAM, { WarpFilter::DRIVE_PARAM }, 9.0f, 1.0f },
        { chowWarp, warpFilter, ChowWarp::HEAT_PARAM, { WarpFilter::FB_PARAM }, 0.9f },
        { chowWarp, warpFilter, ChowWarp::WIDTH_PARAM, { WarpFilter::GAIN_PARAM }, 28.0f, -14.0f },
        { chowWarp, warpFilter, ChowWarp::WIDTH_PARAM, { WarpFilter::Q_PARAM }, -0.85f, 1.0f },
        { chowWarp, warpFilter, ChowWarp::DRIVE_PARAM, { WarpFilter::FB_DRIVE_PARAM }, 9.0f, 1.0f },
    };
}

} // ChowWarpMappings
