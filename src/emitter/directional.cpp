#include <core/object.h>
#include <emitter/emitter.h>
#include <stats/warp.h>
#include <core/scene.h>
#include <bsdf/bsdf.h>
#include <core/common.h>
#include <emitter/directional.h>

NORI_NAMESPACE_BEGIN

DirectionalEmitter::DirectionalEmitter(const PropertyList &props)
{
    radiance = props.getColor("radiance");
    direction = props.getVector("direction").normalized();
}

Color3f DirectionalEmitter::getEmittance(const EmitterQueryRecord &rec) const {
    return radiance;
}

float DirectionalEmitter::pdf(const EmitterQueryRecord& rec, EMeasure unit) const {
    return 1.f;
}

Color3f DirectionalEmitter::evalRadiance(const EmitterQueryRecord &rec, const Scene* scene) const {

    Color3f emitted = getEmittance(rec);
    Color3f bsdf_term = evalBSDF(rec);

    return emitted * bsdf_term;
}

Color3f DirectionalEmitter::sampleRadiance(EmitterQueryRecord &rec, Sampler &sampler, const Scene *scene, float &pdf,
                                    EMeasure unit) const {

    samplePoint(sampler, rec, pdf, EMeasure::ESurfaceArea);

    if(!is_source_visible(scene, rec)) {
        return Color3f(0.f);
    }

    return evalRadiance(rec, scene);
}

void DirectionalEmitter::samplePoint(Sampler &sampler, EmitterQueryRecord &rec, float &pdf, EMeasure unit) const {

    Ray3f dir_ray(rec.p, -direction);
    float nearT, farT;
    scene_bounds.rayIntersect(dir_ray, nearT, farT);

    rec.l = dir_ray.o + farT * dir_ray.d;
    rec.n_l = rec.l; // normal in the same direction as the ray
    pdf = 1.f;
}

void DirectionalEmitter::setParent(NoriObject *parent) {
    scene_bounds = static_cast<Scene*>(parent)->getBoundingBox();
}

float DirectionalEmitter::to_angular(const EmitterQueryRecord& rec, float pdf) const {
    return 1.f;
}

float DirectionalEmitter::angular_distortion(const EmitterQueryRecord &rec) const {
    return 1.f;
}

NORI_NAMESPACE_END

