#include <core/object.h>
#include <stats/sampler.h>
#include <core/scene.h>
#include <emitter/emitter.h>
#include <bsdf/bsdf.h>

NORI_NAMESPACE_BEGIN

class MeshEmitter : public Emitter {

public:

    MeshEmitter(const PropertyList& props);

    float pdf(const EmitterQueryRecord &rec, EMeasure unit) const override;
    Color3f evalRadiance(const EmitterQueryRecord &rec, const Scene* scene) const override;
    Color3f sampleRadiance(EmitterQueryRecord& rec, Sampler& sampler, const Scene* scene, float& angular_pdf) const override;
    Color3f getEmittance(const EmitterQueryRecord &rec) const override;

    void setParent(NoriObject *parent) override;

    std::string toString() const {
        return "MeshEmitter[]";
    }

protected:
    Mesh* mesh;
    Color3f color;

    void samplePoint(Sampler &sampler, EmitterQueryRecord &rec, float &pdf) const override;
};

NORI_REGISTER_CLASS(MeshEmitter, "area")
NORI_NAMESPACE_END