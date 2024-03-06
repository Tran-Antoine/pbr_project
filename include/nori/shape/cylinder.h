#pragma once

#include <core/common.h>
#include <shape/implicit.h>
#include <core/vector.h>

NORI_NAMESPACE_BEGIN

class Cylinder : public ImplicitShape {

public:

    Cylinder(float radius, float height);
    bool rayIntersect(const Ray3f &ray, Intersection &its, bool shadow) const override;

private:
    float radius;
    float height;

    static float Sqr(float x) {
        return x*x;
    }
};
NORI_NAMESPACE_END