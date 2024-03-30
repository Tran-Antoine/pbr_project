#pragma once

#include <core/common.h>
#include <core/object.h>
#include <core/color.h>
#include <bsdf/diffusemap.h>

#include <utility>

NORI_NAMESPACE_BEGIN

class UniformDiffuseMap : public DiffuseMap {

public:
    explicit UniformDiffuseMap(Color3f albedo) : albedo(std::move(albedo)), DiffuseMap("albedo") {}
    explicit UniformDiffuseMap(const PropertyList& propList);
    Color3f T(float s, float t) const override;

    std::string toString() const override;

private:
    Color3f albedo;
};

NORI_NAMESPACE_END