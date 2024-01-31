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
        
        if (!scene->rayIntersect(ray, its)) {
            return Color3f(0.0f);
        }

        Vector3f sampled_point = Warp::squareToCosineHemisphere(sampler->next2D());
        Vector3f w = its.shFrame.toWorld(sampled_point);


        if(scene->rayIntersect(Ray3f(its.p, w))) {
            return Color3f(0.0f);
        }
    
        float cos_theta = its.shFrame.n.dot(w);
        // divide by PDF to make our estimator unbiaised
        float intensity = std::max(0.0f, cos_theta) / (M_PI * (Warp::squareToCosineHemispherePdf(sampled_point)));
    
        return Color3f(intensity);
    }

    std::string toString() const {
        return "AmbientIntegrator[]";
    }
};

NORI_REGISTER_CLASS(AmbientOcclusion, "ao");
NORI_NAMESPACE_END

