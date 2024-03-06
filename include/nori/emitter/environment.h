#pragma once

#include <core/common.h>
#include <emitter/emitter.h>
#include <collection/mipmap.h>
#include <shape/cylinder.h>

NORI_NAMESPACE_BEGIN

class EnvironmentEmitter : public Emitter, public Cylinder {
public:

    explicit EnvironmentEmitter(const PropertyList& props);

    float pdf(const EmitterQueryRecord& rec) const override;
    Color3f evalRadiance(const EmitterQueryRecord &rec, const Scene* scene) const override;
    Color3f sampleRadiance(EmitterQueryRecord& rec, Sampler& sampler, const Scene* scene, float& angular_pdf) const override;
    Color3f getEmittance(const EmitterQueryRecord &rec) const override;

    std::string toString() const {
        return "EnvironmentEmitter[]";
    }


protected:

    bool is_on_map1(const EmitterQueryRecord &rec) const;
    bool is_on_map2(const EmitterQueryRecord &rec) const;

    Point3f map1_to_world(const Point2f& coords) const;
    Point3f map2_to_world(const Point2f& coords) const;
    float weight_map1() const;
    float weight_map2() const { return 1 - weight_map1(); }

    void samplePoint(Sampler &sampler, EmitterQueryRecord &rec, float &pdf) const override;

    MipMap map1, map2;
    Point3f center;
    float radius;
    float height;
};

NORI_REGISTER_CLASS(EnvironmentEmitter, "env")
NORI_NAMESPACE_END