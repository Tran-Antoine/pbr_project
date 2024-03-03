#pragma once

#include <core/common.h>
#include <emitter/emitter.h>
#include <collection/mipmap.h>

NORI_NAMESPACE_BEGIN

class EnvironmentEmitter : public Emitter {
public:

    EnvironmentEmitter(const PropertyList& props);

    float pdf(const EmitterQueryRecord& rec) const override;
    Color3f evalRadiance(EmitterQueryRecord& rec, const Scene* scene) const override;
    Color3f sampleRadiance(EmitterQueryRecord& rec, Sampler& sampler, const Scene* scene, float& angular_pdf) const override;
    Color3f getEmittance(Point3f pos, Vector3f normal, Vector3f direction) const override;

    std::string toString() const {
        return "EnvironmentEmitter[]";
    }

    void setParent(NoriObject *parent) override;

protected:

    bool is_on_map1(const Point3f& p) const;
    bool is_on_map2(const Point3f& p) const;
    Point2i world_to_map1(const Point3f& p) const;
    Point2i world_to_map2(const Point3f& p) const;
    Point3f map1_to_world(const Point2f& coords) const;
    Point3f map2_to_world(const Point2f& coords) const;
    float weight_map1() const;
    float weight_map2() const { return 1 - weight_map1(); }

    MipMap map1, map2;
    Point3f center;
    float radius;
    float height;
};

NORI_REGISTER_CLASS(EnvironmentEmitter, "env")
NORI_NAMESPACE_END