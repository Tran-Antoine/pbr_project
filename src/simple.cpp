#include <nori/integrator.h>
#include <nori/scene.h>
#include <algorithm>

NORI_NAMESPACE_BEGIN

class SimpleIntegrator : public Integrator {
public:
    SimpleIntegrator(const PropertyList &props) {
        position = props.getPoint("position");
        color = props.getColor("energy");
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
        
        Intersection its;
        
        if (!scene->rayIntersect(ray, its))
            return Color3f(0.0f);

        Vector3f point_to_light = position - its.p;   

        Ray3f shadowQuery = Ray3f(its.p, point_to_light);

        if(scene->rayIntersect(shadowQuery)) {
            return Color3f(0.0f);
        }

        float cos_theta = its.shFrame.n.dot(point_to_light.normalized());
        Color3f output_color = color * 1 / (4 * M_PI * M_PI) * std::max(0.0f, cos_theta) / (point_to_light).squaredNorm();
        
        return output_color;
    }

    std::string toString() const {
        return "NormalIntegrator[]";
    }
protected:
    Point3f position;
    Color3f color;
};

NORI_REGISTER_CLASS(SimpleIntegrator, "simple");
NORI_NAMESPACE_END

