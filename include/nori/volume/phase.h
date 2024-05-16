#pragma once

#include <core/common.h>
#include <core/object.h>
#include <core/color.h>

NORI_NAMESPACE_BEGIN

class PhaseFunction : public NoriObject {

public:

    EClassType getClassType() const override { return EPhaseFunction; }

    virtual float pdf(const Vector3f &in, const Vector3f &out) const = 0;
    virtual void sample(Sampler *sampler, const Vector3f &in, Vector3f &out, float &pdf) const = 0;
    virtual Color3f eval(const Vector3f &in, const Vector3f &out) const = 0;

};

class UniformPhase : public PhaseFunction {
public:
    UniformPhase() = default;
    explicit UniformPhase(const PropertyList &props) {}
    float pdf(const Vector3f &in, const Vector3f &out) const override;
    void sample(Sampler *sampler, const Vector3f &in, Vector3f &out, float &pdf) const override;
    Color3f eval(const Vector3f &in, const Vector3f &out) const override;
    std::string toString() const override;

};

class HenyeyGreensteinPhase : public PhaseFunction {

public:
    explicit HenyeyGreensteinPhase(float g) : g(g) {}
    float pdf(const Vector3f &in, const Vector3f &out) const override;
    void sample(Sampler *sampler, const Vector3f &in, Vector3f &out, float &pdf) const override;
    Color3f eval(const Vector3f &in, const Vector3f &out) const override;
    std::string toString() const override;

protected:
    float g;
};

NORI_NAMESPACE_END