#pragma once
#include <core/common.h>
#include <core/color.h>
#include <core/vector.h>
#include <core/mesh.h>

NORI_NAMESPACE_BEGIN

class DiffuseMap {

public:

    virtual Color3f T(float s, float t) const = 0;

    Color3f T(Point2f p) const {
        return T(p.x(), p.y());
    }

};

NORI_NAMESPACE_END