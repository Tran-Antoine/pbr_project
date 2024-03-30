#include <core/common.h>
#include <core/color.h>
#include <bsdf/uniformmap.h>

NORI_NAMESPACE_BEGIN

UniformDiffuseMap::UniformDiffuseMap(const PropertyList& propList)
    : albedo(propList.getColor("albedo"), DiffuseMap(propList)) {}

Color3f UniformDiffuseMap::T(float s, float t) const {
    return albedo;
}

std::string UniformDiffuseMap::toString() const {
    return "uniform";
}

NORI_NAMESPACE_END