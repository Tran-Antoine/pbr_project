#include <nori/integrator.h>
#include <nori/scene.h>
#include <nori/emitter.h>

NORI_NAMESPACE_BEGIN

class WhittedIntegrator : public Integrator {
public:
    WhittedIntegrator(const PropertyList &props) {
        /* No parameters */
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
        /* Find the surface that is visible in the requested direction */
        Intersection its;
        
        if (!scene->rayIntersect(ray, its))
            return Color3f(0.0f);

        // for now we just do one sample
        Color3f radiance;

        Mesh* random_emitter = scene->pickMeshEmitter();

        if(random_emitter) {
            radiance += random_emitter->getEmitter()->computeRadiance(its.mesh->getBSDF(), its.p, its.shFrame.n, -ray.d, *sampler, scene);
        }

        return radiance;

    }

    std::string toString() const {
        return "WhittedIntegrator[]";
    }
};

NORI_REGISTER_CLASS(WhittedIntegrator, "whitted");
NORI_NAMESPACE_END

