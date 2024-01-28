#include <nori/object.h>
#include <nori/sampler.h>
#include <nori/scene.h>
#include <nori/emitter.h>

NORI_NAMESPACE_BEGIN

class MeshEmitter : public Emitter {

public:

    MeshEmitter(const PropertyList& props);

    Color3f computeRadiance(Point3f at, Vector3f at_normal, Vector3f dir, Sampler& sampler, const Scene* scene) override;
    Color3f getEmittance(Point3f pos, Vector3f normal, Vector3f direction) override;
    Color3f getPower() override;

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