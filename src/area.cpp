#include <nori/object.h>
#include <nori/emitter.h>
#include <nori/warp.h>
#include <nori/scene.h>
#include <nori/bsdf.h>
#include <nori/common.h>
#include <nori/area.h>

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

Color3f MeshEmitter::computeRadiance(const BSDF* bsdf, Point3f at, Vector3f at_normal, Vector3f dir_to_camera, Sampler& sampler, const Scene* scene)
{
    // TODO: Add many "Sampler" implementations, one per warping scheme


    // Things to add: 
    // To think about: maybe only sample points whose normal goes against the incoming direction?

    Point2f sample(sampler.next2D());

    Point3f surface_point;
    Vector3f n;
    float pdf;
    Warp::squareToMeshPoint(sample, *mesh, surface_point, n, pdf);

    Vector3f y_to_x = (at - surface_point).normalized();
    Vector3f x_to_y = -y_to_x;
    float distance = (surface_point - at).norm();

    Ray3f ray = Ray3f(at, x_to_y, Epsilon, (1 - Epsilon) * distance);

    if(scene->rayIntersect(ray)) {
        // meaning the ray hit an object BEFORE hitting the light source
        return Color3f(0.0f);
    }

    float jacobian = abs(at_normal.dot(x_to_y) * (n.dot(y_to_x))) / (distance*distance);

    Color3f emitted = getEmittance(surface_point, n, y_to_x);

    Frame local(at_normal);

    BSDFQueryRecord query(local.toLocal(dir_to_camera), local.toLocal(x_to_y), EMeasure::ESolidAngle);
    Color3f bsdf_term = bsdf->eval(query);

    return jacobian/pdf * (emitted * bsdf_term); 
}

NORI_NAMESPACE_END

