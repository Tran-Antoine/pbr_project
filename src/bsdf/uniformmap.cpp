#include <core/common.h>
#include <core/color.h>
#include <bsdf/uniformmap.h>

NORI_NAMESPACE_BEGIN

UniformDiffuseMap::UniformDiffuseMap(const PropertyList& propList)
    : DiffuseMap(propList), albedo(propList.getColor("albedo")) {}

Color3f UniformDiffuseMap::T(float s, float t) const {
    return albedo;
}

std::string UniformDiffuseMap::toString() const {
    return "uniform";
}

NORI_REGISTER_CLASS(UniformDiffuseMap, "uniform");
NORI_NAMESPACE_END