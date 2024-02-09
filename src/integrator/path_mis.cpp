#include <integrator/integrator.h>
#include <emitter/emitter.h>
#include <core/scene.h>
#include <bsdf/bsdf.h>
#include <stats/warp.h>

NORI_NAMESPACE_BEGIN

class MISPathTracer : public Integrator {
public:
    MISPathTracer(const PropertyList &props) {
        q = props.getFloat("q", 0.1f);
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
        
        Color3f Le(0.0f), Ld(0.0f), beta(1.0f), beta_lag(1.0f);

        int bounces = 0;
        bool last_specular = false; 

        Ray3f current_ray = ray;
        const BSDF* surface_bsdf;
        EmitterQueryRecord emitter_rec;
        BSDFQueryRecord bsdf_record;
        float angular_light_pdf; 

        float previous_pdf;

        Point3f previous_p;

        while(sampler->next1D() > q) {

            Intersection its;
            bool found_intersection = scene->rayIntersect(current_ray, its);
            
            if(!found_intersection) {
                break;
            }

            const Point3f x = its.p;
            const Vector3f n = its.shFrame.n;
            const Vector3f wi = -current_ray.d;
            const Emitter* emitter = its.mesh->getEmitter();
            
            
            if(emitter) {
                if(bounces == 0 || last_specular) {
                    Le += beta * emitter->getEmittance(x, n, wi);
                }

                // Add MIS component from the PREVIOUS interaction
                // This can only be done now because during the previous interaction we couldn't know
                // whether the indirect ray intersected with an emitter
                // This whole procedure simulates using two samples and not one, one picked from the BRDF, the other from the light source
                if(surface_bsdf && !bsdf_record.isEmpty()) {
                    float brdf_pdf = previous_pdf;
                    float light_pdf = emitter->pdf(EmitterQueryRecord(surface_bsdf, previous_p, Vector3f(), Vector3f(), x, n));
                    float weight = brdf_pdf / (brdf_pdf + light_pdf);
                    Ld += beta_lag * weight * emitter->evalRadiance(emitter_rec, scene);
                }

                break;
                
            }
            
            Frame frame(n);

            previous_p = x;
            beta_lag = beta;
            surface_bsdf = its.mesh->getBSDF(); // delayed initialization so that the lines above use the previous bsdf
            emitter_rec = EmitterQueryRecord(surface_bsdf, x, n, wi);

            // Direct illumination component
            for(Mesh* mesh : scene->getMeshEmitters()) {
                
                Color3f direct_rad = mesh->getEmitter()->sampleRadiance(emitter_rec, *sampler, scene, angular_light_pdf);
                Vector3f wo = emitter_rec.wo();

                float pdf_light = angular_light_pdf;
                float pdf_brdf  = surface_bsdf->pdf(BSDFQueryRecord(frame.toLocal(wi), frame.toLocal(emitter_rec.wo()), EMeasure::ESolidAngle));
                float weight = pdf_light / (pdf_light + pdf_brdf);

                Ld += beta * weight * direct_rad;
            }

            // Indirect illumination (computing the next step)
            bsdf_record = BSDFQueryRecord(frame.toLocal(wi));
            Color3f bsdf_term = surface_bsdf->sample(bsdf_record, sampler->next2D());

            if(bsdf_term.isZero()) {
                break;
            } 

            previous_pdf = surface_bsdf->pdf(bsdf_record);

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

private:
    float q;

};

NORI_REGISTER_CLASS(MISPathTracer, "path_mis");
NORI_NAMESPACE_END

