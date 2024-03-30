#pragma once

#include <core/object.h>
#include <core/common.h>
#include <core/color.h>
#include <bsdf/diffusemap.h>
#include <ImfRgbaFile.h>
#include <ImfArray.h>

NORI_NAMESPACE_BEGIN

class TextureDiffuseMap : public DiffuseMap {

public:
    explicit TextureDiffuseMap(const PropertyList &propList);
    Color3f T(float s, float t) const override;
    std::string toString() const override;

private:
    int width, height;
    Imf::Array2D<Imf::Rgba> pixels;
};

NORI_NAMESPACE_END