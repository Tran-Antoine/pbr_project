#include <core/common.h>
#include <shape/implicit.h>
#include <core/vector.h>
#include <shape/sphere.h>
#include <emitter/emitter.h>

NORI_NAMESPACE_BEGIN

Sphere::Sphere(float radius) : radius(radius) {}

static float Sqr(float x) {
    return x*x;
}

static float SafeACos(float x) {
    return acos(clamp(x, -1.f, 1.f));
}

bool Sphere::rayIntersect(const nori::Ray3f &ray, nori::Intersection &its, bool shadow) const {
    // Algorithm used by https://pbr-book.org/4ed/Shapes/Spheres#IntersectionTests
    Vector3f di = ray.d;
    Point3f oi = ray.o;

    float phi;
    Point3f intersection;

    float t0, t1;

    float a = Sqr(di.x()) + Sqr(di.y()) + Sqr(di.z());
    float b = 2 * (di.x() * oi.x() + di.y() * oi.y() + di.z() * oi.z());
    float c = Sqr(oi.x()) + Sqr(oi.y()) + Sqr(oi.z()) - Sqr(radius);

    Vector3f vec(oi - b / (2 * a) * di);
    float length = vec.norm();
    float discrim = 4 * a * (float(radius) + length) * (float(radius) - length);
    if (discrim < 0) {
        return false;
    }

    float rootDiscrim = sqrt(discrim);
    float q;
    if (b < 0) {
        q = -.5f * (b - rootDiscrim);
    } else {
        q = -.5f * (b + rootDiscrim);
    }

    t0 = q / a;
    t1 = c / q;

    if (t0 > t1) {
        std::swap(t0, t1);
    }

    if (t0 > ray.maxt || t1 <= 0) {
        return false;
    }

    float t = t0;
    if (t <= 0 || t != t) {
        t = t1;
        if (t > ray.maxt || t != t) {
            return false;
        }
    }

    if (shadow) {
        return true;
    }



    intersection = oi + t * di;

    if (intersection.x() == 0 && intersection.z() == 0) intersection.x() = 1e-5f * radius;
    phi = std::atan2(intersection.z(), intersection.x());

    float u = (phi/M_PI + 1) / 2.f;
    float cosTheta = intersection.y() / radius;
    float theta = SafeACos(cosTheta);
    float v = clamp(theta / M_PI, 0.f, 1.f);

    if(u < 0 || u > 1 || v < 0 || v > 1) {
        throw NoriException("UV parameters out of range");
    }

    Frame normal(-intersection.normalized());

    its.p = intersection;
    its.t = t;
    its.uv = Point2f(u, v);
    its.shFrame = its.geoFrame = normal;
    its.mesh = nullptr;
    its.emitter = dynamic_cast<const Emitter*>(this);

    return true;
}
NORI_NAMESPACE_END