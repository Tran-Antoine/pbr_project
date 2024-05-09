#pragma once

#include <core/object.h>
#include <core/common.h>
#include <core/color.h>
#include <bsdf/diffusemap.h>
#include <ImfRgbaFile.h>
#include <ImfArray.h>

NORI_NAMESPACE_BEGIN

class MultiDiffuseMap : public DiffuseMap {

public:
    explicit MultiDiffuseMap(const PropertyList &propList);
    Color3f T(float s, float t) override;
    float map(float s, int index) const;
    void unmap(float m, float& s, int& index) const;

    std::string toString() const override;
    void addChild(nori::NoriObject *obj) override;

private:
    std::vector<DiffuseMap*> children;
};

NORI_NAMESPACE_END