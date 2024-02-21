#include <core/common.h>
#include <core/color.h>
#include <bsdf/diffusemap.h>
#include <bsdf/uniformmap.h>

NORI_NAMESPACE_BEGIN

UniformDiffuseMap::UniformDiffuseMap(Color3f albedo) : albedo(albedo) {}

Color3f UniformDiffuseMap::T(float s, float t) const {
    return albedo;
}

NORI_NAMESPACE_END