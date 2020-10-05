#pragma once

#include <rack.hpp>

/**
 * Utility class to control mapping parameters between modules
 */
class ParamMap {
public:
    using ParamMapSet = std::vector<ParamMap>;

    ParamMap(rack::Module& sourceModule, rack::Module& destModule, int sourceParam,
        std::vector<int> destParams = {}, float scale = 1.0f, float offset = 0.0f) :
        sourceModule (sourceModule),
        destModule (destModule),
        sourceParam (sourceParam),
        destParams (destParams),
        scale (scale),
        offset (offset)
    {}

    /** Adds the destination parameter to the mapping, if not already present */
    void addDestParam(int paramID) {
        // if param already present
        if(std::find(destParams.begin(), destParams.end(), paramID) != destParams.end())
            return;

        destParams.push_back(paramID);
    }

    /** Removes the destination parameter from the mapping, if it is present */
    void removeDestParam(int paramID) {
        auto iter = std::find(destParams.begin(), destParams.end(), paramID);
        if(iter == destParams.end())
            return;
        
        destParams.erase(iter);
    }

    void setScale(float newScale) { scale = newScale; }
    void setOffset(float newOffset) { offset = newOffset; }

    /** Applies the parameter mapping to the relevant modules */
    inline void apply() {
        auto paramVal = sourceModule.params[sourceParam].getValue() * scale + offset;

        for(auto paramID : destParams)
            destModule.params[paramID].setValue(paramVal);
    }

    static void applySet(ParamMapSet& set) {
        for(auto& map : set)
            map.apply();
    }

private:
    rack::Module& sourceModule;
    rack::Module& destModule;
    const int sourceParam;
    std::vector<int> destParams;
    float scale = 1.0f;
    float offset = 0.0f;
};
