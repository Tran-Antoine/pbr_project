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
            return Color3f(0.7f); // black background is a bit sad

        // problem: accel doesnt support multiple meshes rn

        // for now we just do one sample
        Color3f radiance;

        for(auto mesh : scene->getMeshes()) {
            Emitter* emitter = mesh->getEmitter();
            if(emitter) {
                radiance += emitter->computeRadiance(its.p, -ray.d, *sampler, scene);
            }
        }

        return radiance;

    }

    std::string toString() const {
        return "WhittedIntegrator[]";
    }
};

NORI_REGISTER_CLASS(WhittedIntegrator, "whitted");
NORI_NAMESPACE_END

