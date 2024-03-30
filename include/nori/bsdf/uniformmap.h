#include <core/common.h>
#include <core/color.h>
#include <bsdf/diffusemap.h>
#include <core/object.h>

NORI_NAMESPACE_BEGIN

class UniformDiffuseMap : public DiffuseMap {

public:
    UniformDiffuseMap(const PropertyList& propList);
    Color3f T(float s, float t) const override;

    std::string toString() const override;

private:
    Color3f albedo;
};

NORI_REGISTER_CLASS(UniformDiffuseMap, "uniform");
NORI_NAMESPACE_END