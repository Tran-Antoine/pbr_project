#pragma once

#include <core/common.h>
#include <core/object.h>
#include <core/vector.h>
#include <volume/phase.h>

NORI_NAMESPACE_BEGIN

class Medium;

struct MediumInteraction {

    /// Interval of interaction
    float mint, maxt;
    /// Pointer to the associated mesh
    const Medium* medium;
    /// Create an uninitialized interaction record
    MediumInteraction() : mint(std::numeric_limits<float>::infinity()), maxt(-1.f), medium(nullptr) { }

    bool is_present() const { return medium != nullptr; }
};

using DirecFun = float (*)(const Point3f&, const Vector3f&);

class Medium : public NoriObject {

public:

    EClassType getClassType() const override { return EMedium; }

    float attenuation(const Point3f& p, const Vector3f& v) { return (*absorption)(p, v) + (*scattering)(p, v); }
    float in_scattering(const Point3f& p, const Vector3f& v) { return (*scattering)(p, v); }
    float out_scattering(const Point3f& p, const Vector3f& v) { return (*scattering)(p, v); }
    float emission(const Point3f& p, const Vector3f& v) { return (*absorption)(p, v); }
    void samplePhase(const Vector3f& in, Vector3f& out, float& pdf) { phase->eval(in, out, pdf); }

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
                throw NoriException("Mesh::addChild(<%s>) is not supported!",
                                    classTypeName(obj->getClassType()));
        }
    }

protected:
    PhaseFunction* phase = nullptr;
    DirecFun* absorption = nullptr;
    DirecFun* scattering = nullptr;
};

NORI_NAMESPACE_END