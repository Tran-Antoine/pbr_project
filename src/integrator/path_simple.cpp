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
            const Emitter* hit_emitter = find_emitter(its);
            const BSDF* surface_bsdf = its.mesh ? its.mesh->getBSDF() : nullptr;
            
            if(hit_emitter) {
                if(bounces == 0 || last_specular) {
                    // here x and n are actually l and n_l
                    EmitterQueryRecord emitter_rec = EmitterQueryRecord(nullptr, 0.f, 0.f, wi, x, n, its.uv);
                    Color3f emittance = hit_emitter->getEmittance(emitter_rec);
                    Le += beta * emittance;
                }
                // seems more logic to break the indirect lighting loop whenever we hit an hit_emitter,
                // regardless of whether its contribution should be counted or not
                break;
            }

            if(!surface_bsdf) {
                // It should be impossible for a non-emitter hit to not have a surface BSDF
                throw NoriException("Unreachable statement reached");
            }
            
            // Direct illumination component
            for(Emitter* emitter : scene->getEmitters()) {
                EmitterQueryRecord emitter_rec(surface_bsdf, x, n, wi, its.uv);
                float pdf;
                Color3f direct_rad = emitter->sampleRadiance(emitter_rec, *sampler, scene, pdf, ESurfaceArea);

                if (pdf != 0) {
                    Color3f weighted_rad = direct_rad / pdf;
                    Ld += beta * direct_rad / pdf;
                }
            }


            Frame frame(n);
            BSDFQueryRecord record(frame.toLocal(wi));

            Color3f bsdf_term = surface_bsdf->sample(record, sampler->next2D());

            if(bsdf_term.isZero()){
                break;
            }

            beta = beta * bsdf_term / (1-q);

            current_ray = Ray3f(x, frame.toWorld(record.wo));
            last_specular = !surface_bsdf->isDiffuse();
            bounces++;
        }
        return (Le + Ld);
    }

    static const Emitter* find_emitter(const Intersection& its) {
        if(its.emitter) {
            return its.emitter;
        }
        if(its.mesh) {
            return its.mesh->getEmitter();
        }
        return nullptr;
    }

    std::string toString() const {
        return "SimplePathTracerIntegrator[]";
    }
};

NORI_REGISTER_CLASS(SimplePathTracer, "path_simple");
NORI_NAMESPACE_END