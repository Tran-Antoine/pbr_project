#include <core/common.h>
#include <shape/implicit.h>
#include <core/vector.h>
#include <shape/cylinder.h>
#include <emitter/emitter.h>

NORI_NAMESPACE_BEGIN

Cylinder::Cylinder(float radius, float height) : radius(radius), height(height) {}

bool Cylinder::rayIntersect(const nori::Ray3f &ray, nori::Intersection &its, bool shadow) const {
    // Algorithm used by https://pbr-book.org/4ed/Shapes/Cylinders#IntersectionTests
    Vector3f di = ray.d;
    Point3f oi = ray.o;

    float t0, t1;

    float a = Sqr(di.x()) + Sqr(di.z());
    float b = 2 * (di.x() * oi.x() + di.z() * oi.z());
    float c = Sqr(oi.x()) + Sqr(oi.z()) - Sqr((radius));

    float f = b / (2 * a);
    float vx = oi.x() - f * di.x(), vy = oi.z() - f * di.z();
    float length = sqrt(Sqr(vx) + Sqr(vy));
    float discrim = 4 * a * ((radius) + length) *
                    ((radius) - length);
    if (discrim < 0) {
        return false;
    }

    float rootDiscrim = sqrt(discrim);
    float q;
    if (b < 0) q = -.5f * (b - rootDiscrim);
    else q = -.5f * (b + rootDiscrim);
    t0 = q / a;
    t1 = c / q;

    if (t0 > t1)
        std::swap(t0, t1);

    if (t1 <= 0) {
        return false;
    }

    float t = t0;
    if (t <= 0) {
        t = t1;
    }

    if(t > ray.maxt) {
        return false;
    }

    Point3f intersection = oi + t * di;

    if (intersection.y() > height / 2 || intersection.y() < -height / 2) {
        return false;
    }

    if (shadow) {
        return true;
    }
    float x = intersection.x() / radius, z = intersection.z() / radius;

    float u, v;

    u = (atan2(z, x) / M_PI + 1) / 2.f;
    v = (intersection.y() + height / 2.f) / height;

    if(u < 0 || u > 1) {
        throw NoriException("");
    }

    Frame normal(Vector3f(-intersection.x(), 0.f, -intersection.z()).normalized());


    its.p = intersection;
    its.t = t;
    its.uv = Point2f(u, v);
    its.shFrame = its.geoFrame = normal;
    its.mesh = nullptr;
    its.emitter = dynamic_cast<const Emitter*>(this);

    return true;
}
NORI_NAMESPACE_END