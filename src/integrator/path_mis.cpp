#include <integrator/integrator.h>
#include <emitter/emitter.h>
#include <core/scene.h>
#include <bsdf/bsdf.h>
#include <stats/warp.h>

NORI_NAMESPACE_BEGIN

class MISPathTracer : public Integrator {
public:
    MISPathTracer(const PropertyList &props) {
        q = props.getFloat("q", 0.05f);
    }

    static Color3f direct(Color3f beta, float weight, Color3f emittance) {
        return weight * (beta * emittance);
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
        
        Color3f Le(0.0f), Ld(0.0f), beta(1.0f);

        int bounces = 0;
        bool last_specular = false; 

        Ray3f current_ray = ray;
        Intersection its;
        bool found_intersection = scene->rayIntersect(current_ray, its);

        while(sampler->next1D() > q) {
            
            if(!found_intersection) {
                break;
            }

            const Point3f x = its.p;
            const Vector3f n = its.shFrame.n;
            const Vector3f wi = -current_ray.d;
            const Emitter* emitter = its.mesh->getEmitter();
            const BSDF* surface_bsdf = its.mesh->getBSDF(); 
            Frame frame(n);
            EmitterQueryRecord emitter_rec = EmitterQueryRecord(surface_bsdf, x, n, wi);


            if(emitter) {
                if(bounces == 0 || last_specular) {
                    Le += beta * emitter->getEmittance(x, n, wi);
                }
                break;
            }


            // Direct illumination with Emitter Importance Sampling
            for(Mesh* mesh : scene->getMeshEmitters()) {
                
                float pdf_light;
                Color3f direct_rad = mesh->getEmitter()->sampleRadiance(emitter_rec, *sampler, scene, pdf_light);
                Vector3f wo = emitter_rec.wo();

                float pdf_brdf  = surface_bsdf->pdf(BSDFQueryRecord(wi, wo, frame, EMeasure::ESolidAngle));
                float weight = balancedMIS(pdf_light, pdf_brdf);

                Ld += direct(beta, weight, direct_rad);
            }

            // Indirect illumination (computing the next step)
            BSDFQueryRecord bsdf_record = BSDFQueryRecord(wi, frame);
            Color3f bsdf_term = surface_bsdf->sample(bsdf_record, sampler->next2D());

            if(bsdf_term.isZero()) { break; } 
            
            current_ray = Ray3f(x, frame.toWorld(bsdf_record.wo));
            found_intersection = scene->rayIntersect(current_ray, its);
            beta = beta * bsdf_term / (1-q);
            last_specular = !surface_bsdf->isDiffuse();
            bounces++;

            // Direct illumination with BRDF Importance Sampling
            // Done late as we need the collision result from the next step first
            if(found_intersection && its.mesh->getEmitter()) {
                
                const Emitter* emitter = its.mesh->getEmitter();

                float light_pdf = emitter->pdf(EmitterQueryRecord(surface_bsdf, x, n, wi, its.p, its.shFrame.n));
                float brdf_pdf = surface_bsdf->pdf(bsdf_record);
                float weight = balancedMIS(brdf_pdf, light_pdf);

                Ld += direct(beta, weight, emitter->getEmittance(its.p, its.shFrame.n, -current_ray.d));
            }
        }

        return Le + Ld;
    }

    std::string toString() const {
        return "MISPathTracerIntegrator[]";
    }

private:
    float q;

};

NORI_REGISTER_CLASS(MISPathTracer, "path_mis");
NORI_NAMESPACE_END

