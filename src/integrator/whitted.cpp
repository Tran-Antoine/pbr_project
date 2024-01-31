#include <integrator/integrator.h>
#include <core/scene.h>
#include <emitter/emitter.h>
#include <algorithm>

NORI_NAMESPACE_BEGIN

class WhittedIntegrator : public Integrator {
public:
    WhittedIntegrator(const PropertyList &props) {
        /* No parameters */
    }

    static Color3f clip(Color3f i) {
        return Color3f(std::clamp(i.x(), 0.0f, 1.0f), std::clamp(i.y(), 0.0f, 1.0f), std::clamp(i.z(), 0.0f, 1.0f));
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
        /* Find the surface that is visible in the requested direction */
        Intersection its;
        
        if (!scene->rayIntersect(ray, its))
            return Color3f(0.0f);

        if(its.mesh->getEmitter()) {
            return Color3f(0.0f);
        }

        const BSDF* bsdf = its.mesh->getBSDF();

        if(bsdf->isDiffuse()) {
            Color3f radiance;

            Mesh* random_emitter = scene->pickMeshEmitter();

            if(random_emitter) {
                radiance += random_emitter->getEmitter()->sampleRadiance(its.mesh->getBSDF(), its.p, its.shFrame.n, -ray.d, *sampler, scene);
            }

            return clip(radiance);

        } else {
            float eps = sampler->next1D();
            if(eps > 0.95) {
                return Color3f(0.0f);
            }

            BSDFQueryRecord record(its.shFrame.toLocal(-ray.d));
            Color3f sampled_value = bsdf->sample(record, sampler->next2D());

            return clip(1 / 0.95 * sampled_value * Li(scene, sampler, Ray3f(its.p, its.shFrame.toWorld(record.wo)))); 
        }
        

    }

    std::string toString() const {
        return "WhittedIntegrator[]";
    }
};

NORI_REGISTER_CLASS(WhittedIntegrator, "whitted");
NORI_NAMESPACE_END

