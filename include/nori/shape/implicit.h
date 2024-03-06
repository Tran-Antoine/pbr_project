#pragma once

#include "core/common.h"
#include "core/mesh.h"

NORI_NAMESPACE_BEGIN

class ImplicitShape {

public:
    virtual bool rayIntersect(const Ray3f &ray, Intersection &its, bool shadow) const = 0;
};

NORI_NAMESPACE_END
