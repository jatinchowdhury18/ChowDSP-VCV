#pragma once

#include <rack.hpp>

struct ChoiceQuantity : public rack::ParamQuantity {
    float getDisplayValue() override {
        float v = getSmoothValue();
        int choice = static_cast<int> (v);
        choice = rack::clamp(choice, 0, (int) maxValue - 1);
        return (float) choice;
    }
};
