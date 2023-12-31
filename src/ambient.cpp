#include <nori/integrator.h>
#include <nori/scene.h>
#include <algorithm>
#include <nori/sampler.h>
#include <nori/warp.h>
#include <algorithm>

NORI_NAMESPACE_BEGIN

class AmbientOcclusion : public Integrator {
public:

    AmbientOcclusion(const PropertyList &props) {}

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
        
        Intersection its;
        
        if (!scene->rayIntersect(ray, its))
            return Color3f(0.0f);

        float intensity = 0;

        for(int i = 0; i < sampler->getSampleCount(); i++) {

            Vector3f w = Warp::squareToCosineHemisphere(sampler->next2D());

            w = its.shFrame.toWorld(w);

            if(scene->rayIntersect(Ray3f(its.p, w))) {
                continue;
            }
        
            float cos_theta = its.shFrame.n.dot(w);

            intensity += std::max(0.0f, cos_theta);
        }

        intensity /= (M_PI * sampler->getSampleCount());

        return Color3f(intensity);
    }

    std::string toString() const {
        return "AmbientIntegrator[]";
    }
};

NORI_REGISTER_CLASS(AmbientOcclusion, "ao");
NORI_NAMESPACE_END

