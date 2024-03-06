#include <core/scene.h>
#include <core/ray.h>
#include <emitter/emitter.h>


NORI_NAMESPACE_BEGIN

bool Emitter::is_source_visible(const Scene* scene, const EmitterQueryRecord& rec) const {
    Ray3f ray = Ray3f(rec.p, rec.wo(), Epsilon, (1 - Epsilon) * rec.dist());
    return scene->rayIntersect(ray);
}

float Emitter::angular_distortion(const EmitterQueryRecord &rec) const {
    float dist = rec.dist();
    Vector3f wo = rec.wo();
    return abs(rec.n_p.dot(wo) * (rec.n_l.dot(wo))) / (dist*dist);
}

Color3f Emitter::evalBSDF(const EmitterQueryRecord &rec) const {
    Frame frame(rec.n_p); // BSDFQueryRecord expects local vectors
    BSDFQueryRecord query(frame.toLocal(rec.wi), frame.toLocal(rec.wo()), EMeasure::ESolidAngle, rec.uv);
    return rec.bsdf->eval(query);
}

float Emitter::to_angular(const EmitterQueryRecord &rec, float pdf) const {
    float dist = rec.dist();
    return pdf * dist * dist / rec.n_p.dot(rec.wo());
}

NORI_NAMESPACE_END