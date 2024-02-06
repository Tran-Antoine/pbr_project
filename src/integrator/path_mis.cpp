#include <integrator/integrator.h>
#include <emitter/emitter.h>
#include <core/scene.h>
#include <bsdf/bsdf.h>
#include <stats/warp.h>

NORI_NAMESPACE_BEGIN

class MISPathTracer : public Integrator {
public:
    MISPathTracer(const PropertyList &props) {
        
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
        
        Color3f Le = Color3f(0.0f), Ld = Color3f(0.0f);

        int bounces = 0;
        bool last_specular = false;

        Color3f beta(1.0f);

        Ray3f current_ray = ray;

        float q = 0.1;

        const BSDF* surface_bsdf;
        EmitterQueryRecord emitter_rec;
        BSDFQueryRecord bsdf_record;
        float angular_light_pdf; 

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
            
            
            if(emitter) {
                if(bounces == 0 || last_specular) {
                    Le += beta * emitter->getEmittance(x, n, wi);
                }

                // Add MIS component from the PREVIOUS interaction
                // This can only be done now because during the previous interaction we couldn't know
                // whether the indirect ray intersected with an emitter
                // This whole procedure simulates using two samples and not one, one picked from the BRDF, the other from the light source
                if(surface_bsdf && !bsdf_record.isEmpty() && !emitter_rec.isEmpty()) {
                    float brdf_pdf = surface_bsdf->pdf(bsdf_record);
                    float light_pdf = angular_light_pdf;
                    float weight = brdf_pdf / (brdf_pdf + light_pdf);
                    Ld += beta * emitter->evalRadiance(emitter_rec, scene);
                }
                
            }
            
            Frame frame(n);

            surface_bsdf = its.mesh->getBSDF(); // delayed initialization so that the lines above use the previous bsdf
            bsdf_record = BSDFQueryRecord(frame.toLocal(wi));
            

            // Direct illumination component
            for(Mesh* mesh : scene->getMeshEmitters()) {
                emitter_rec = EmitterQueryRecord(surface_bsdf, x, n, wi);
                Color3f direct_rad = mesh->getEmitter()->sampleRadiance(emitter_rec, *sampler, scene, angular_light_pdf);

                float pdf_light = angular_light_pdf;
                float pdf_brdf  = surface_bsdf->pdf(BSDFQueryRecord(bsdf_record.wi, emitter_rec.wo(), EMeasure::ESolidAngle));
                float weight = pdf_light / (pdf_light + pdf_brdf);

                Ld += beta * weight * direct_rad;
            }

            // Indirect illumination (computing the next step)
            Color3f bsdf_term = surface_bsdf->sample(bsdf_record, sampler->next2D()); 
            beta = beta * bsdf_term / (1-q);
            current_ray = Ray3f(x, frame.toWorld(bsdf_record.wo));
            last_specular = !surface_bsdf->isDiffuse();
            bounces++;
        }
        return (Le + Ld);
    }

    std::string toString() const {
        return "MISPathTracerIntegrator[]";
    }
};

NORI_REGISTER_CLASS(MISPathTracer, "path_mis");
NORI_NAMESPACE_END

