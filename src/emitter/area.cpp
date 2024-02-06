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

Color3f MeshEmitter::getEmittance(Point3f pos, Vector3f normal, Vector3f direction) const {
    
    if(normal.dot(direction) < 0) return Color3f(0.0f);

    return color;
}

float MeshEmitter::pdf(EmitterQueryRecord& rec) const {
    float d2 = (rec.l - rec.p).squaredNorm();
    float cos_theta = rec.n_l.dot((rec.p - rec.l).normalized());
    float area = mesh->getTotalArea();

    return d2 / (area * cos_theta);
}

Color3f MeshEmitter::evalRadiance(EmitterQueryRecord& rec, const Scene* scene) const {
    
    Vector3f x_to_y = (rec.l - rec.p).normalized();
    float distance = (rec.l - rec.p).norm();

    // determinant of the jacobian of the change of coordinates
    float distortion_factor = abs(rec.n_p.dot(x_to_y) * (rec.n_l.dot(x_to_y))) / (distance*distance);

    Color3f emitted = getEmittance(rec.l, rec.n_l, -x_to_y);

    Frame frame(rec.n_p); // BSDFQueryRecord expects local vectors
    BSDFQueryRecord query(frame.toLocal(rec.wi), frame.toLocal(x_to_y), EMeasure::ESolidAngle);

    Color3f bsdf_term = rec.bsdf->eval(query);

    return distortion_factor * (emitted * bsdf_term); 
}

Color3f MeshEmitter::sampleRadiance(EmitterQueryRecord& rec, Sampler& sampler, const Scene* scene, float& angular_pdf) const {

    // TODO: get rid of duplication with evalRadiance
    Point2f sample(sampler.next2D());

    Point3f light_point;
    Vector3f light_n;
    float pdf_light;
    Warp::squareToMeshPoint(sample, *mesh, light_point, light_n, pdf_light);


    Vector3f x_to_y = (light_point - rec.p).normalized();
    float distance = (light_point - rec.p).norm();

    // We stop the ray right before its intersection with the light source (which would be guaranteed to happen)
    Ray3f ray = Ray3f(rec.p, x_to_y, Epsilon, (1 - Epsilon) * distance);

    if(scene->rayIntersect(ray)) {
        // meaning the ray hit an object BEFORE hitting the light source
        return Color3f(0.0f);
    }

    // determinant of the jacobian of the change of coordinates
    float distortion_factor = abs(rec.n_p.dot(x_to_y) * (light_n.dot(x_to_y))) / (distance*distance);

    Color3f emitted = getEmittance(light_point, light_n, -x_to_y);

    Frame frame(rec.n_p); // BSDFQueryRecord expects local vectors
    BSDFQueryRecord query(frame.toLocal(rec.wi), frame.toLocal(x_to_y), EMeasure::ESolidAngle);

    Color3f bsdf_term = rec.bsdf->eval(query);

    angular_pdf = pdf_light * distance * distance / light_n.dot(-x_to_y);
    rec.l = light_point;
    rec.n_l = light_n;

    return distortion_factor * (emitted * bsdf_term) / pdf_light; 
}

NORI_NAMESPACE_END

