#include <core/common.h>
#include <core/color.h>
#include <bsdf/diffusemap.h>


NORI_NAMESPACE_BEGIN

class UniformDiffuseMap : public DiffuseMap {

public:
    UniformDiffuseMap(Color3f albedo);
    Color3f T(float s, float t) const override;

private:
    Color3f albedo;
};

NORI_NAMESPACE_END