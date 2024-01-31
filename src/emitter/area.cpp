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

Color3f MeshEmitter::getEmittance(Point3f pos, Vector3f normal, Vector3f direction) {
    return color;
}

Color3f MeshEmitter::sampleRadiance(const BSDF* bsdf, Point3f p, Vector3f n, Vector3f wi, Sampler& sampler, const Scene* scene)
{
    // TODO: Add many "Sampler" implementations, one per warping scheme

    Point2f sample(sampler.next2D());

    Point3f light_point;
    Vector3f light_n;
    float pdf;
    Warp::squareToMeshPoint(sample, *mesh, light_point, light_n, pdf);

    Vector3f x_to_y = (light_point - p).normalized();
    float distance = (light_point - p).norm();

    // We stop the ray right before its intersection with the light source (which would be guaranteed to happen)
    Ray3f ray = Ray3f(p, x_to_y, Epsilon, (1 - Epsilon) * distance);

    if(scene->rayIntersect(ray)) {
        // meaning the ray hit an object BEFORE hitting the light source
        return Color3f(0.0f);
    }

    // determinant of the jacobian of the change of coordinates
    float distortion_factor = abs(n.dot(x_to_y) * (light_n.dot(x_to_y))) / (distance*distance);

    Color3f emitted = getEmittance(light_point, light_n, -x_to_y);

    Frame frame(n); // BSDFQueryRecord expects local vectors
    BSDFQueryRecord query(frame.toLocal(wi), frame.toLocal(x_to_y), EMeasure::ESolidAngle);

    Color3f bsdf_term = bsdf->eval(query);

    return distortion_factor/pdf * (emitted * bsdf_term); 
}

NORI_NAMESPACE_END

