#include <core/object.h>
#include <emitter/emitter.h>
#include <stats/warp.h>
#include <core/scene.h>
#include <bsdf/bsdf.h>
#include <core/common.h>
#include <emitter/area.h>

NORI_NAMESPACE_BEGIN

MeshEmitter::MeshEmitter(const PropertyList &props)
{
    color = props.getColor("radiance");
}

void MeshEmitter::setParent(NoriObject *parent) {
    mesh = static_cast<Mesh*>(parent);
}

Color3f MeshEmitter::getEmittance(const EmitterQueryRecord &rec) const {
    
    if(rec.n_l.dot(-rec.wo()) < 0) {
        return Color3f(0.0f);
    }

    return color;
}

float MeshEmitter::pdf(const EmitterQueryRecord& rec, EMeasure unit) const {
    float pdf = 1.f / mesh->getTotalArea();
    switch(unit) {
        case EMeasure::ESurfaceArea: return pdf;
        case EMeasure::ESolidAngle: return to_angular(rec, pdf);
        default: throw NoriException("Unsupported unit");
    }
}

Color3f MeshEmitter::evalRadiance(const EmitterQueryRecord &rec, const Scene* scene) const {

    Color3f emitted = getEmittance(rec);
    Color3f bsdf_term = evalBSDF(rec);

    // determinant of the jacobian of the change of coordinates
    return angular_distortion(rec) * (emitted * bsdf_term);
}

Color3f MeshEmitter::sampleRadiance(EmitterQueryRecord &rec, Sampler &sampler, const Scene *scene, float &pdf,
                                    EMeasure unit) const {

    float pdf_light;
    samplePoint(sampler, rec, pdf_light, EMeasure::ESurfaceArea);

    if (unit == EMeasure::ESolidAngle) {
        pdf = to_angular(rec, pdf_light);
    } else {
        pdf = pdf_light;
    }

    if(!is_source_visible(scene, rec)) {
        return Color3f(0.f);
    }

    // determinant of the jacobian of the change of coordinates
    return evalRadiance(rec, scene);
}

void MeshEmitter::samplePoint(Sampler &sampler, EmitterQueryRecord &rec, float &pdf, EMeasure unit) const {
    Point2f sample(sampler.next2D());
    Warp::squareToMeshPoint(sample, *mesh, rec.l, rec.n_l, pdf);
}

NORI_NAMESPACE_END

