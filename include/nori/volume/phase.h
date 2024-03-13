#pragma once

#include <core/common.h>
#include <core/object.h>

NORI_NAMESPACE_BEGIN

class PhaseFunction : public NoriObject {

public:

    EClassType getClassType() const override { return EPhaseFunction; }

    virtual void eval(Sampler *sampler, const Vector3f &in, Vector3f &out, float &pdf) const = 0;

};

class UniformPhase : public PhaseFunction {
    void eval(Sampler *sampler, const Vector3f &in, Vector3f &out, float &pdf) const override;
    std::string toString() const override;
};

NORI_NAMESPACE_END