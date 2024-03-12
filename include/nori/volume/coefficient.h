#pragma once

#include <core/common.h>
#include <core/object.h>
#include <core/vector.h>
#include <volume/phase.h>

NORI_NAMESPACE_BEGIN

class MediumCoefficient {

public:
    float eval(const Point3f& p, const Vector3f& v) { return 0.f; }
};

NORI_NAMESPACE_END
