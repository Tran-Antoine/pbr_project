#include <integrator/integrator.h>
#include <core/scene.h>
#include <stats/warp.h>
#include <volume/medium.h>

NORI_NAMESPACE_BEGIN

// TODO: Rename this: it's only for homogeneous volumetric scattering
class VolumetricIntegrator : public Integrator {
public:
    VolumetricIntegrator(const PropertyList &props) {
        /* No parameters this time */
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {

        Intersection its;
        bool found_intersection = scene->rayIntersect(ray, its);

        if(found_intersection && its.mesh->getEmitter()) {
            EmitterQueryRecord emitter_rec = EmitterQueryRecord(nullptr, 0.f, 0.f, -ray.d, its.p, its.shFrame.n, its.uv);
            return its.mesh->getEmitter()->getEmittance(emitter_rec);
        }

        if(its.medium.is_present()) {
            const Medium* medium = its.medium.medium;
            float omega_t = medium->attenuation(0.f, 0.f);
            float omega_s = medium->in_scattering(0.f, 0.f);

            float maxt = its.medium.maxt;
            float t = Warp::lineToHomogeneousPath(sampler->next1D(), omega_t);

            if(t < maxt) {
                Point3f p = ray.o + t * ray.d;
                Vector3f direction;
                float new_direction_pdf;
                medium->samplePhase(sampler, ray.d, direction, new_direction_pdf);

                return omega_s / omega_t * Li(scene, sampler, Ray3f(p, ray.d));

            } else if (found_intersection) {

                if(its.mesh->getEmitter()) {
                    EmitterQueryRecord emitter_rec = EmitterQueryRecord(nullptr, 0.f, 0.f, -ray.d, its.p, its.shFrame.n, its.uv);
                    return its.mesh->getEmitter()->getEmittance(emitter_rec);
                }

                BSDFQueryRecord rec(its.shFrame.toLocal(-ray.d), its.uv);
                Color3f bsdf_term = its.mesh->getBSDF()->sample(rec, sampler->next2D());
                Vector3f next_direction = its.shFrame.toWorld(rec.wo);

                return bsdf_term * Li(scene, sampler, Ray3f(its.p, next_direction));
            }

            return Color3f(0.f);


        } else if(found_intersection) {
            if(its.mesh->getEmitter()) {
                EmitterQueryRecord emitter_rec = EmitterQueryRecord(nullptr, 0.f, 0.f, -ray.d, its.p, its.shFrame.n, its.uv);
                return its.mesh->getEmitter()->getEmittance(emitter_rec);
            }

            BSDFQueryRecord rec(its.shFrame.toLocal(-ray.d), its.uv);
            Color3f bsdf_term = its.mesh->getBSDF()->sample(rec, sampler->next2D());
            Vector3f next_direction = its.shFrame.toWorld(rec.wo);

            return bsdf_term * Li(scene, sampler, Ray3f(its.p, next_direction));

        } else {
            return Color3f(0.f);

        }
    }

    std::string toString() const {
        return "VolumetricIntegrator[]";
    }
};

NORI_REGISTER_CLASS(VolumetricIntegrator, "volumetric");
NORI_NAMESPACE_END

