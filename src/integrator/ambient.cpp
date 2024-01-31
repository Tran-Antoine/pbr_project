#include <integrator/integrator.h>
#include <core/scene.h>
#include <algorithm>
#include <stats/sampler.h>
#include <stats/warp.h>
#include <algorithm>

NORI_NAMESPACE_BEGIN

class AmbientOcclusion : public Integrator {
public:

    AmbientOcclusion(const PropertyList &props) {}

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
        
        Intersection its;
        
        if (!scene->rayIntersect(ray, its))
            return Color3f(0.0f);


        Vector3f w = Warp::squareToCosineHemisphere(sampler->next2D());

        w = its.shFrame.toWorld(w);

        if(scene->rayIntersect(Ray3f(its.p, w))) {
            return 0;
        }
    
        float cos_theta = its.shFrame.n.dot(w);

        float intensity = std::max(0.0f, cos_theta) / M_PI;
    
        return Color3f(intensity);
    }

    std::string toString() const {
        return "AmbientIntegrator[]";
    }
};

NORI_REGISTER_CLASS(AmbientOcclusion, "ao");
NORI_NAMESPACE_END

