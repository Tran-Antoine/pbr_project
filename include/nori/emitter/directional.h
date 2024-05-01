#pragma once

#include <core/object.h>
#include <stats/sampler.h>
#include <core/scene.h>
#include <emitter/emitter.h>
#include <bsdf/bsdf.h>

NORI_NAMESPACE_BEGIN

class DirectionalEmitter : public Emitter {

public:

    DirectionalEmitter(const PropertyList& props);

    float pdf(const EmitterQueryRecord &rec, EMeasure unit) const override;
    Color3f evalRadiance(const EmitterQueryRecord &rec, const Scene* scene) const override;
    Color3f sampleRadiance(EmitterQueryRecord &rec, Sampler &sampler, const Scene *scene, float &pdf,
                           EMeasure unit) const override;
    Color3f getEmittance(const EmitterQueryRecord &rec) const override;

    float to_angular(const EmitterQueryRecord& rec, float pdf) const override;
    float angular_distortion(const EmitterQueryRecord& rec) const override;

    void setParent(NoriObject *parent) override;

    void samplePoint(Sampler &sampler, EmitterQueryRecord &rec, float &pdf, EMeasure unit) const override;

    std::string toString() const {
        return "DirectionalEmitter[]";
    }

protected:
    Color3f radiance;
    Vector3f direction;
    BoundingBox3f scene_bounds;
};

NORI_REGISTER_CLASS(DirectionalEmitter, "directional")
NORI_NAMESPACE_END