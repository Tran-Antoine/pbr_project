#include <core/object.h>
#include <stats/sampler.h>
#include <core/scene.h>
#include <emitter/emitter.h>
#include <bsdf/bsdf.h>

NORI_NAMESPACE_BEGIN

class MeshEmitter : public Emitter {

public:

    MeshEmitter(const PropertyList& props);

    Color3f sampleRadiance(const BSDF* bsdf, Point3f at, Vector3f at_normal, Vector3f dir, Sampler& sampler, const Scene* scene) const override;
    Color3f getEmittance(Point3f pos, Vector3f normal, Vector3f direction) const override;

    void setParent(NoriObject *parent) override;

    std::string toString() const {
        return "MeshEmitter[]";
    }

protected:
    Mesh* mesh;
    Color3f color;
};

NORI_REGISTER_CLASS(MeshEmitter, "area")
NORI_NAMESPACE_END