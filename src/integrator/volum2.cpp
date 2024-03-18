#include <integrator/integrator.h>
#include <core/scene.h>
#include <stats/warp.h>
#include <volume/medium.h>

NORI_NAMESPACE_BEGIN

class Volum2Integrator : public Integrator {
public:
    explicit Volum2Integrator(const PropertyList &props) {
        /* No parameters this time */
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {

        Intersection its;
        bool found = scene->rayIntersect(ray, its);

        if(its.medium.is_present()) {
            return mediumFound(its, sampler, found);
        } else if(found) {
            return emitterFound(its);
        } else {
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

    static Color3f mediumFound(const Intersection& its, Sampler* sampler, bool solidIntersection) {

        float t_max = solidIntersection ? its.t : std::numeric_limits<float>::infinity();
        const Medium* medium = its.medium.medium;
        float omega_t = medium->attenuation(0.f, 0.f);
        float omega_s = medium->in_scattering(0.f, 0.f);

        float t_sample = Warp::lineToHomogeneousPath(sampler->next1D(), omega_t);

        if(t_sample < t_max) {

        } else {

        }
    }

    static Color3f emitterFound(const Intersection& its) {
        const Emitter* emitter = its.emitter ? its.emitter : its.mesh->getEmitter();
        if(!emitter) {
            throw NoriException("Only emitter objects are supported in volum2");
        }
        return emitter->getEmittance(EmitterQueryRecord(nullptr, 0.f, 0.f, 0.f, its.p, its.shFrame.n, its.uv));
    }

    std::string toString() const {
        return "Volum1Integrator[]";
    }
};

NORI_REGISTER_CLASS(Volum2Integrator, "volum2");
NORI_NAMESPACE_END

