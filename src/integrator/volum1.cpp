#include <integrator/integrator.h>
#include <core/scene.h>
#include <stats/warp.h>
#include <volume/medium.h>

NORI_NAMESPACE_BEGIN

class Volum1Integrator : public Integrator {
public:
    explicit Volum1Integrator(const PropertyList &props) {
        /* No parameters this time */
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {

        Intersection its;
        bool found = scene->rayIntersect(ray, its);

        if(!found) {
            return Color3f(0.f);
        }

        const Emitter* emitter = its.emitter ? its.emitter : its.mesh->getEmitter();

        if(!emitter) {
            std::cout << "Unreachable code: V1 only supports emitter meshes";
            return Color3f(1.f, 0.f, 0.f);
        }

        Color3f emitted = emitter->getEmittance(EmitterQueryRecord(nullptr, 0.f, 0.f, 0.f, its.p, its.shFrame.n, its.uv));

        if(!its.medium.is_present()) {
            return emitted;
        }

        float mint = its.medium.mint, maxt = its.medium.maxt;
        float omega_t = its.medium.medium->attenuation(0.f, 0.f);
        float transmittance = exp(-omega_t * (maxt - mint));

        return transmittance * emitted;
    }

    std::string toString() const {
        return "Volum1Integrator[]";
    }
};

NORI_REGISTER_CLASS(Volum1Integrator, "volum1");
NORI_NAMESPACE_END

