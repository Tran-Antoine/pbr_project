#include <integrator/integrator.h>
#include <emitter/emitter.h>
#include <core/scene.h>
#include <bsdf/bsdf.h>
#include <stats/warp.h>

NORI_NAMESPACE_BEGIN

class SimplePathTracer : public Integrator {
public:
    SimplePathTracer(const PropertyList &props) {
        
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
        
        Color3f Le = Color3f(0.0f), Ld = Color3f(0.0f);

        int bounces = 0;
        bool last_specular = false;

        Color3f beta(1.0f);

        Ray3f current_ray = ray;

        float q = 0.1;

        while(sampler->next1D() > q) {

            Intersection its;
            bool found_intersection = scene->rayIntersect(current_ray, its);
            
            if(!found_intersection) {
                break;
            }

            Point3f x = its.p;
            Vector3f n = its.shFrame.n;
            Vector3f wi = -current_ray.d;
            const Emitter* emitter = its.mesh->getEmitter();
            const BSDF* surface_bsdf = its.mesh->getBSDF();
            
            if(emitter) {
                if(bounces == 0 || last_specular) {
                    Le += beta * emitter->getEmittance(x, n, wi);
                }
                // seems more logic to break the indirect lighting loop whenever we hit an emitter,
                // regardless of whether its contribution should be counted or not
                break;
            }
            
            // Direct illumination component
            for(Mesh* mesh : scene->getMeshEmitters()) {
                EmitterQueryRecord emitter_rec(surface_bsdf, x, n, wi);
                float unused;
                Color3f direct_rad = mesh->getEmitter()->sampleRadiance(emitter_rec, *sampler, scene, unused);

                Ld += beta * direct_rad; 
            }


            Frame frame(n);
            BSDFQueryRecord record(frame.toLocal(wi));

            Color3f bsdf_term = surface_bsdf->sample(record, sampler->next2D()); 

            if(!bsdf_term.isValid()) {
                //std::cout << typeid(*surface_bsdf).name() << "\n";
            }
            beta = beta * bsdf_term / (1-q);

            current_ray = Ray3f(x, frame.toWorld(record.wo));
            last_specular = !surface_bsdf->isDiffuse();
            bounces++;
        }
        return (Le + Ld);
    }

    std::string toString() const {
        return "SimplePathTracerIntegrator[]";
    }
};

NORI_REGISTER_CLASS(SimplePathTracer, "path_simple");
NORI_NAMESPACE_END