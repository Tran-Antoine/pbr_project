#include <integrator/integrator.h>
#include <core/scene.h>
#include <emitter/emitter.h>

NORI_NAMESPACE_BEGIN

class EnvOnlyIntegrator : public Integrator {
public:
    EnvOnlyIntegrator(const PropertyList &props) {
        radius = props.getFloat("radius");
        height = props.getFloat("height");
    }

    static float Sqr(float x) {return x*x;}

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {

        // Algorithm used by https://pbr-book.org/4ed/Shapes/Cylinders#IntersectionTests
        Vector3f di = ray.d;
        Point3f oi = ray.o;

        float t0, t1;

        float a = Sqr(di.x()) + Sqr(di.z());
        float b = 2 * (di.x() * oi.x() + di.z() * oi.z());
        float c = Sqr(oi.x()) + Sqr(oi.z()) - Sqr((radius));

        float f = b / (2 * a);
        float vx = oi.x() - f * di.x(), vy = oi.z() - f * di.z();
        float length = sqrt(Sqr(vx) + Sqr(vy));
        float discrim = 4 * a * ((radius) + length) *
                           ((radius) - length);
        if (discrim < 0)
            return Color3f(0.f);

        float rootDiscrim = sqrt(discrim);
        float  q;
        if (b < 0) q = -.5f * (b - rootDiscrim);
        else              q = -.5f * (b + rootDiscrim);
        t0 = q / a;
        t1 = c / q;

        if (t0 > t1)
            std::swap(t0, t1);

        if (t1 <= 0)
            return Color3f(0.f);

        float t = t0;
        if (t <= 0) {
            t = t1;
        }

        Point3f intersection = oi + t*di;

        if(intersection.y() > height / 2 || intersection.y() < -height/2) {
            return Color3f(0.f);
        }

        Emitter* emitter = scene->getEmitters()[0];

        return emitter->getEmittance(intersection, 0.f, 0.f);
    }

    std::string toString() const {
        return "NormalIntegrator[]";
    }
private:
    float radius;
    float height;
};

NORI_REGISTER_CLASS(EnvOnlyIntegrator, "env_only");
NORI_NAMESPACE_END

