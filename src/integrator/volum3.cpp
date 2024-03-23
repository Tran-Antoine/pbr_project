#include <integrator/integrator.h>
#include <core/scene.h>
#include <stats/warp.h>
#include <volume/medium.h>

NORI_NAMESPACE_BEGIN

class Volum3Integrator : public Integrator {
public:
    explicit Volum3Integrator(const PropertyList &props) {
        /* No parameters this time */
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {

        float q = 0.1;

        Color3f beta(1.f);
        Ray3f current_ray = ray;

        while(sampler->next1D() > q) {

            Intersection its; bool found = scene->rayIntersect(current_ray, its);

            // If no medium is hit, don't bother with transmittance sampling, etc.
            if(!its.medium.is_present()) {
                return nonVolumetricTracing(found, its, beta);
            }

            // TODO: fix that if the camera is inside a medium and the intersection is found, no medium intersection is detected
            float t_max = maxt(found, its);
            const Medium* medium = its.medium.medium;
            float omega_t = medium->attenuation(0.f, 0.f);
            float omega_s = medium->in_scattering(0.f, 0.f);

            // sample distance until scattering
            float t_through_media = Warp::lineToHomogeneousPath(sampler->next1D(), omega_t);
            float t_traveled = its.medium.mint + t_through_media;

            // If true, handle scattering event
            if(t_traveled < t_max) {
                if(omega_s == 0.f) return Color3f(0.f); // short circuit to speed up

                Point3f scattering_point = current_ray.o + t_traveled * current_ray.d;
                Vector3f next_direction; float unused;
                medium->samplePhase(sampler, current_ray.d, next_direction, unused);

                current_ray = Ray3f(scattering_point, next_direction);
                // integral over all directions of phase function of luminance must be 1, so the
                // normalization factor cancels out the pdf factor
                beta *= omega_s / (omega_t * (1-q));

            } else if(found){
                // transmittance/pdf = 1, both cancel out
                beta *= exp(-omega_t * t_through_media);
                return nonVolumetricTracing(found, its, beta);
            } else {
                return Color3f(0.f);
            }
        }
        // Didn't hit a surface after n rebounds, just returning 0
        return Color3f(0.f);
    }

    static float maxt(bool found, const Intersection& its) {
        return found ? its.t : std::numeric_limits<float>::infinity();
    }

    static Color3f nonVolumetricTracing(bool found, const Intersection& its, const Color3f& beta) {

        if(!found) {
            return Color3f(0.f);
        }
        // TODO: divide by t^2 ? By cos between wi and n_l ? Both ?
        return beta * emittance(its);
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

NORI_REGISTER_CLASS(Volum3Integrator, "volum3");
NORI_NAMESPACE_END

