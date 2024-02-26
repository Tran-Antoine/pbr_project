#include <integrator/integrator.h>
#include <core/scene.h>
#include <bsdf/bsdf.h>

NORI_NAMESPACE_BEGIN

/// Integrator mainly useful for debugging purposes. Calls BSDF::eval assuming wo doesn't matter and can be the 0 vector
class DirectIntegrator : public Integrator {
public:
    DirectIntegrator(const PropertyList &props) {
        /* No parameters this time */
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
        /* Find the surface that is visible in the requested direction */
        Intersection its;

        if (!scene->rayIntersect(ray, its))
            return Color3f(0.0f);

        /* Return the component-wise absolute
           value of the shading normal as a color */
        const BSDF* bsdf = its.mesh->getBSDF();

        BSDFQueryRecord record(-ray.d, Vector3f(0.f, 0.f, 1.f), EMeasure::ESolidAngle);
        return bsdf->eval(record);
    }

    std::string toString() const {
        return "NormalIntegrator[]";
    }
};

NORI_REGISTER_CLASS(DirectIntegrator, "direct");
NORI_NAMESPACE_END

