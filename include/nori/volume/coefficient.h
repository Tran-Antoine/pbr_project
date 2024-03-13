#pragma once

#include <core/common.h>
#include <core/object.h>
#include <core/vector.h>
#include <volume/phase.h>

NORI_NAMESPACE_BEGIN

class MediumCoefficient {

public:
    virtual float eval(const Point3f& p, const Vector3f& v) const { return 0.f; }
};

class ConstantCoefficient : public MediumCoefficient {
public:
    explicit ConstantCoefficient(float coeff) : coeff(coeff) {}

    float eval(const Point3f& p, const Vector3f& v) const { return coeff; }
private:
    float coeff;
};

NORI_NAMESPACE_END
