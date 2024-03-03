#include <integrator/integrator.h>
#include <core/scene.h>
#include <emitter/emitter.h>

NORI_NAMESPACE_BEGIN

class EnvOnlyIntegrator : public Integrator {
public:
    EnvOnlyIntegrator(const PropertyList &props) {
        radius = props.getFloat("radius");
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
        
        Point3f intersection = ray.o + radius * ray.d.normalized();

        Emitter* emitter = scene->getEmitters()[0];

        return emitter->getEmittance(intersection, 0.f, 0.f);
    }

    std::string toString() const {
        return "NormalIntegrator[]";
    }
private:
    float radius;
};

NORI_REGISTER_CLASS(EnvOnlyIntegrator, "env_only");
NORI_NAMESPACE_END

