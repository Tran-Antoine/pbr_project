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

        if(its.medium.is_present()) {
            const Medium* medium = its.medium.medium;

            float maxt = found_intersection ? its.t : std::numeric_limits<float>::infinity();
            float t = Warp::lineToHomogeneousPath(sampler->next1D(), medium->attenuation(0.f, 0.f));

            if(t < maxt) {

            }
            return Color3f(0.f);
        } else if(found_intersection) {
            return Color3f(0.f); // TODO: sample BRDF
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

