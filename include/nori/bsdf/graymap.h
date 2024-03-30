#pragma once

#include <core/common.h>
#include <core/color.h>
#include <bsdf/diffusemap.h>
#include <ImfRgbaFile.h>
#include <ImfArray.h>

NORI_NAMESPACE_BEGIN

class GrayMap : public DiffuseMap {

public:
    explicit GrayMap(const DiffuseMap* child) : child(child), DiffuseMap("graymap") {}

    virtual Color3f map(float gray) const = 0;

    Color3f T(float s, float t) const override {

        float gray = child->T(s, t).r();
        return map(gray);
    }

private:
    const DiffuseMap* child;
};

class IdentityMap : public GrayMap {

    explicit IdentityMap(const DiffuseMap* child) : GrayMap(child) {}

    Color3f map(float gray) const override {
        return Color3f(gray);
    }
};

NORI_NAMESPACE_END