#pragma once

#include <core/common.h>
#include <core/object.h>
#include <core/color.h>
#include <bsdf/diffusemap.h>

#include <utility>
#include <pcg32.h>

NORI_NAMESPACE_BEGIN

class RandomLuminosityDiffuseMap : public DiffuseMap {

public:
    explicit RandomLuminosityDiffuseMap(int hue, float saturation, float min_lum, float max_lum)
    : hue(hue), saturation(saturation), min_lum(min_lum), max_lum(max_lum), DiffuseMap("luminosity") {}

    explicit RandomLuminosityDiffuseMap(const PropertyList& propList);
    Color3f T(float s, float t) override;

    std::string toString() const override;

protected:
    int hue;
    float saturation, min_lum, max_lum;
    pcg32 random;
};

NORI_NAMESPACE_END