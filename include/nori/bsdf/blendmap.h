#pragma once

#include <core/object.h>
#include <core/common.h>
#include <core/color.h>
#include <bsdf/diffusemap.h>
#include <ImfRgbaFile.h>
#include <ImfArray.h>

NORI_NAMESPACE_BEGIN

class BlendMap : public DiffuseMap {

public:
    explicit BlendMap(const PropertyList& propList);
    Color3f T(float s, float t) const override;
    std::string toString() const override;

    void addChild(nori::NoriObject *child) override;

private:
    int width, height;
    Imf::Array2D<Imf::Rgba> pixels;
    DiffuseMap* red_map = nullptr;
    DiffuseMap* green_map = nullptr;
    DiffuseMap* blue_map = nullptr;
};

NORI_REGISTER_CLASS(BlendMap, "uniform");
NORI_NAMESPACE_END