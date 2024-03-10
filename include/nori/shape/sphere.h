#pragma once

#include <core/common.h>
#include <shape/implicit.h>
#include <core/vector.h>

NORI_NAMESPACE_BEGIN

class Sphere : public ImplicitShape {

public:

    Sphere(float radius);
    bool rayIntersect(const Ray3f &ray, Intersection &its, bool shadow) const override;

private:
    float radius;
    float height;

    static float Sqr(float x) {
        return x*x;
    }
};
NORI_NAMESPACE_END