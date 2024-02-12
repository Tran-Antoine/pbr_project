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
        
        Color3f Le(0.0f), Ld(0.0f), beta(1.0f);

        int bounces = 0;
        bool last_specular = false; 

        Ray3f current_ray = ray;
        const BSDF* surface_bsdf;
        EmitterQueryRecord emitter_rec;
        BSDFQueryRecord bsdf_record;

        Intersection its;
        bool found_intersection;

        while(sampler->next1D() > q) {

            if(bounces == 0) {
                found_intersection = scene->rayIntersect(current_ray, its);
            }
            
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

                break;
            }
            
            Frame frame(n);


            surface_bsdf = its.mesh->getBSDF(); // delayed initialization so that the lines above use the previous bsdf
            emitter_rec = EmitterQueryRecord(surface_bsdf, x, n, wi);

            // Direct illumination component
            for(Mesh* mesh : scene->getMeshEmitters()) {
                
                float pdf_light;
                Color3f direct_rad = mesh->getEmitter()->sampleRadiance(emitter_rec, *sampler, scene, pdf_light);
                Vector3f wo = emitter_rec.wo();

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
            
            current_ray = Ray3f(x, frame.toWorld(bsdf_record.wo));

            found_intersection = scene->rayIntersect(current_ray, its);
            
            beta = beta * bsdf_term / (1-q);

            if(found_intersection) {
                if(its.mesh->getEmitter()) {
                    float light_pdf = its.mesh->getEmitter()->pdf(EmitterQueryRecord(surface_bsdf, x, n, wi, its.p, its.shFrame.n));
                    float brdf_pdf = surface_bsdf->pdf(bsdf_record);
                    float weight = brdf_pdf / (brdf_pdf + light_pdf);
                    Ld += beta * weight * its.mesh->getEmitter()->getEmittance(its.p, its.shFrame.n, -current_ray.d);
                }
            }


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

