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
            return mediumFound(scene, ray, its, sampler, found);
        } else if(found) {
            return emittance(its);
        } else {
            return Color3f(0.f);
        }
    }

    static Color3f mediumFound(const Scene* scene, const Ray3f& ray, const Intersection& its, Sampler* sampler, bool solidIntersection) {

        float t_max = solidIntersection ? its.t : std::numeric_limits<float>::infinity();
        const Medium* medium = its.medium.medium;
        float omega_t = medium->attenuation(0.f, 0.f);
        float omega_s = medium->in_scattering(0.f, 0.f);

        // sample distance until scattering
        float t_through_media = Warp::lineToHomogeneousPath(sampler->next1D(), omega_t);
        float t_traveled = its.medium.mint + t_through_media;

        if(t_traveled < t_max) {

            float travel_pdf = Warp::lineToHomogeneousPathPdf(t_through_media, omega_t); // pdf of going to that distance (continuous PDF)
            float transmittance = exp(-omega_t * t_through_media);
            Point3f scattering_point = ray.o + t_traveled * ray.d;

            // sample directly on the emitters
            Color3f out(0.f);

            for(const Emitter* emitter : scene->getEmitters()) {

                Vector3f wi  = -ray.d;

                EmitterQueryRecord record(nullptr, scattering_point, 0.f, wi, 0.f);
                float light_point_pdf;
                emitter->samplePoint(*sampler, record, light_point_pdf, EMeasure::ESolidAngle);
                Color3f phase_term = medium->evalPhase(wi, record.wo());
                Color3f emitted = emitter->getEmittance(record);

                if(light_point_pdf != 0.f && emitter->is_source_visible(scene, record)) {
                    // transmittance/pdf = 1/omega_t
                    out += emitted * omega_s * phase_term / (light_point_pdf * omega_t);
                }
            }
            return out;
        } else if(solidIntersection){
            // transmittance/pdf = 1, both cancel out
            return emittance(its);
        } else {
            return Color3f(0.f);
        }
    }

    static Color3f emittance(const Intersection& its) {
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

