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

        const BSDF* bsdf = its.mesh->getBSDF();

        if(bsdf->isDiffuse()) {
            Color3f radiance;

            Mesh* random_emitter = scene->pickMeshEmitter();

            if(random_emitter) {
                radiance += random_emitter->getEmitter()->computeRadiance(its.mesh->getBSDF(), its.p, its.shFrame.n, -ray.d, *sampler, scene);
            }

            return radiance;

        } else {
            float eps = sampler->next1D();
            if(eps > 0.95) {
                return Color3f(0.0f);
            }

            BSDFQueryRecord record(its.shFrame.toLocal(-ray.d));
            Color3f sampled_value = bsdf->sample(record, sampler->next2D());

            return 1 / 0.95 * sampled_value * Li(scene, sampler, Ray3f(its.p, its.shFrame.toWorld(record.wo))); 
        }
        

    }

    std::string toString() const {
        return "WhittedIntegrator[]";
    }
};

NORI_REGISTER_CLASS(WhittedIntegrator, "whitted");
NORI_NAMESPACE_END

