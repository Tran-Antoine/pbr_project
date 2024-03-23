#include <integrator/integrator.h>
#include <core/scene.h>
#include <stats/warp.h>
#include <volume/medium.h>

NORI_NAMESPACE_BEGIN

class Volum4Integrator : public Integrator {
public:
    explicit Volum4Integrator(const PropertyList &props) {
        /* No parameters this time */
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {

        if(ray.d.isApprox(Vector3f(0.0223538522, -0.0804889798, 0.996504902))) {
            std::cout << "y";
        }
        float q = 0.1;

        Color3f Le(0.f), Li(0.f);
        Color3f beta(1.f);
        Ray3f current_ray = ray;

        int bounces = 0;

        while(sampler->next1D() > q) {

            // short circuit to avoid computation if the contribution is zero
            if(beta.isZero()) {
                break;
            }

            Intersection its; bool found = scene->rayIntersect(current_ray, its);
            Vector3f wi = -current_ray.d;
            const Emitter* hit_emitter = find_emitter(its);

            if(hit_emitter) {
                if(!its.medium.is_present() && bounces == 0) {
                    Le += beta * emittance(its);
                }

                if (bounces != 0) {
                    break;
                }
            }

            // If no medium and no emitter is hit, the ray just got lost in the void
            if(!its.medium.is_present()) {
                break;
            }

            // Next event estimation calculation
            const Medium* medium = its.medium.medium;
            float omega_t = medium->attenuation(0.f, 0.f);
            float omega_s = medium->in_scattering(0.f, 0.f);
            float t_max = maxt(found, its);
            float t_through_media = Warp::lineToHomogeneousPath(sampler->next1D(), omega_t);
            float t_traveled = its.medium.mint + t_through_media;

            if(t_traveled >= t_max) {
                beta *= exp(-omega_t * t_through_media);
                Le += beta * nonVolumetricTracing(found, its);
                break;
            }


            Point3f scattering_point = current_ray.o + t_traveled * current_ray.d;

            for(const Emitter* emitter : scene->getEmitters()) {

                EmitterQueryRecord record(nullptr, scattering_point, 0.f, wi, 0.f);
                float light_point_pdf;
                emitter->samplePoint(*sampler, record, light_point_pdf, EMeasure::ESolidAngle);
                Color3f phase_term = medium->evalPhase(wi, record.wo());
                Color3f emitted = emitter->getEmittance(record);

                if(light_point_pdf != 0.f && emitter->is_source_visible(scene, record)) {
                    // transmittance/pdf = 1
                    Li += beta * emitted * omega_s * phase_term / light_point_pdf;
                }
            }

            // Handle scattering event
            Vector3f next_direction; float unused;
            medium->samplePhase(sampler, current_ray.d, next_direction, unused);
            // TODO: Same problem here: the ray starts from within the medium, so if it hits the emitter it wont know that there is an emitter
            // and the if(!its.medium.is_present()) will be true
            current_ray = Ray3f(scattering_point, next_direction);
            current_ray.starting_medium = medium;
            beta *= omega_s * medium->evalPhase(0.f, 0.f); // constant phase function
            bounces++;
        }
        return Le + Li;
    }

    static const Emitter* find_emitter(const Intersection& its) {
        if(its.emitter) {
            return its.emitter;
        }
        if(its.mesh) {
            return its.mesh->getEmitter();
        }
        return nullptr;
    }

    static float maxt(bool found, const Intersection& its) {
        return found ? its.t : std::numeric_limits<float>::infinity();
    }

    static Color3f nonVolumetricTracing(bool found, const Intersection& its) {

        if(!found) {
            return Color3f(0.f);
        }
        // TODO: divide by t^2 ? By cos between wi and n_l ? Both ?
        return emittance(its);
    }


    static Color3f emittance(const Intersection& its) {
        const Emitter* emitter = its.emitter ? its.emitter : its.mesh->getEmitter();
        if(!emitter) {
            throw NoriException("Only emitter objects are supported in volum4");
        }
        return emitter->getEmittance(EmitterQueryRecord(nullptr, 0.f, 0.f, 0.f, its.p, its.shFrame.n, its.uv));
    }

    std::string toString() const {
        return "Volum1Integrator[]";
    }
};

NORI_REGISTER_CLASS(Volum4Integrator, "volum4");
NORI_NAMESPACE_END

