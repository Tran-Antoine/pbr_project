#pragma once

#include <core/common.h>
#include <core/object.h>
#include <core/vector.h>
#include <volume/phase.h>
#include <volume/coefficient.h>
#include <core/bbox.h>
#include <core/color.h>

NORI_NAMESPACE_BEGIN

struct MediumInteraction {

    /// Interval of interaction
    float mint, maxt;
    /// Pointer to the associated mesh
    const Medium* medium;
    /// Create an uninitialized interaction record
    MediumInteraction() : mint(std::numeric_limits<float>::infinity()), maxt(-1.f), medium(nullptr) { }

    bool is_present() const { return medium != nullptr; }
};

class Medium : public NoriObject {

public:

    EClassType getClassType() const override { return EMedium; }

    Color3f attenuation(const Point3f& p, const Vector3f& v) const { return absorption->eval(p, v) + scattering->eval(p, v); }
    Color3f in_scattering(const Point3f& p, const Vector3f& v) const { return scattering->eval(p, v); }
    Color3f out_scattering(const Point3f& p, const Vector3f& v) const { return scattering->eval(p, v); }
    Color3f emission(const Point3f& p, const Vector3f& v) const { return absorption->eval(p, v); }
    void samplePhase(Sampler* sampler, const Vector3f& in, Vector3f& out, float& pdf) const { phase->sample(sampler, in, out, pdf); }
    Color3f evalPhase(const Vector3f& in, const Vector3f& out) const { return phase->eval(in, out); }
    float phasePdf(const Vector3f& in, const Vector3f& out) const { return phase->pdf(in, out); }
    virtual BoundingBox3f bounds() const { return BoundingBox3f(); }
    virtual float majorant(const Ray3f& ray) const { return absorption->maj(ray) + scattering->maj(ray);}
    /// Register a child object (e.g. a BSDF) with the mesh
    void addChild(NoriObject *obj) override {
        switch (obj->getClassType()) {
            case NoriObject::EPhaseFunction:
                if(phase) {
                    throw NoriException("Medium: tried to register multiple Phase instances!");
                }
                phase = static_cast<PhaseFunction *>(obj);
                break;
            default:
                throw NoriException("Medium::addChild(<%s>) is not supported!",
                                    classTypeName(obj->getClassType()));
        }
    }

    MediumCoefficient* getAbsorption() const { return absorption; }
    MediumCoefficient* getScattering() const { return scattering; }

protected:
    PhaseFunction* phase = nullptr;
    MediumCoefficient* scattering = nullptr;
    MediumCoefficient* absorption = nullptr;
};

NORI_NAMESPACE_END