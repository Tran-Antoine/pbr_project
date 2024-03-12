#pragma once

#include <core/common.h>
#include <core/object.h>

NORI_NAMESPACE_BEGIN

class PhaseFunction : public NoriObject {

public:

    EClassType getClassType() const override { return EPhaseFunction; }

    virtual void eval(const Vector3f& in, Vector3f& out, float& pdf) = 0;

protected:

};

NORI_NAMESPACE_END