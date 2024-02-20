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

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
        /* Find the surface that is visible in the requested direction */
        Intersection its;
        
        if (!scene->rayIntersect(ray, its)) {
            return Color3f(0.0f);
        }

        const BSDF* bsdf = its.mesh->getBSDF();
        Frame frame = its.shFrame;

        if(bsdf->isDiffuse()) {

            Mesh* random_emitter = scene->pickMeshEmitter();

            if(random_emitter) {
                // crucial to use the total emitters area and NOT the area of the selected mesh, as the PDF of the sampled points depends on the total area
                EmitterQueryRecord record(its.mesh->getBSDF(), its.p, frame.n, -ray.d);
                float unused;
                Color3f sampled_radiance = random_emitter->getEmitter()->sampleRadiance(record, *sampler, scene, unused);
                return sampled_radiance; // 1/pdf = total area
            }

            return Color3f(0.0f);

            // Alternative : sample all light sources
            /*Color3f test;

            for(Mesh* mesh : scene->getMeshes()) {
                if(mesh->getEmitter()) {
                    test += mesh->getEmitter()->sampleRadiance(its.mesh->getBSDF(), its.p, frame.n, -ray.d, *sampler, scene) * mesh->getTotalArea();
                }
            }

            return test;*/

        } else {

            float eps = sampler->next1D();
            if(eps >= 0.95) {
                return Color3f(0.0f);
            }

            BSDFQueryRecord record(frame.toLocal(-ray.d));
            Color3f sampled_value = bsdf->sample(record, sampler->next2D());

            return (1.0 / 0.95) * sampled_value * Li(scene, sampler, Ray3f(its.p, frame.toWorld(record.wo))); 
        }
    }

    std::string toString() const {
        return "WhittedIntegrator[]";
    }
};

NORI_REGISTER_CLASS(WhittedIntegrator, "whitted");
NORI_NAMESPACE_END

