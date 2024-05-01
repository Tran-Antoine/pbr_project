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

    static Color3f direct(const Color3f& beta, float weight, const Color3f& emittance) {
        Color3f out = weight * (beta * emittance);
        return out;
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
            const Emitter* hit_emitter = find_emitter(its);
            const BSDF* surface_bsdf = its.mesh ? its.mesh->getBSDF() : nullptr;
            Frame frame(n);
            EmitterQueryRecord emitter_rec = EmitterQueryRecord(surface_bsdf, x, n, wi, its.uv);


            if(hit_emitter) {
                if(bounces == 0 || last_specular) {
                    EmitterQueryRecord hit_emitter_rec = EmitterQueryRecord(nullptr, ray.o, 0.f, wi, x, n, its.uv);
                    Color3f emittance = hit_emitter->getEmittance(hit_emitter_rec);

                    Le += beta * emittance;
                }
                //break;
            }

            if(!surface_bsdf) {
                // It should be impossible for a non-emitter hit to not have a surface BSDF
                throw NoriException("Unreachable statement reached");
            }

            // Direct illumination with Emitter Importance Sampling
            for(Emitter* emitter : scene->getEmitters()) {

                if(!emitter) {
                    throw NoriException("Null emitter found in the scene");
                }

                float pdf_light;
                float pdf_light_angular;

                Color3f direct_rad = emitter->sampleRadiance(emitter_rec, *sampler, scene, pdf_light, EMeasure::ESurfaceArea);

                if(pdf_light == 0) {
                    continue;
                }
                direct_rad /= pdf_light; // TODO
                pdf_light_angular = emitter->to_angular(emitter_rec, pdf_light);

                Vector3f wo = emitter_rec.wo();

                float pdf_brdf  = surface_bsdf->pdf(BSDFQueryRecord(wi, wo, frame, EMeasure::ESolidAngle));
                float weight = balancedMIS(pdf_light_angular, pdf_brdf);

                Ld += direct(beta, weight, direct_rad);
            }

            // Indirect illumination (computing the next step)
            BSDFQueryRecord bsdf_record = BSDFQueryRecord(wi, frame, its.uv);
            Color3f bsdf_term = surface_bsdf->sample(bsdf_record, sampler->next2D());

            if(bsdf_term.isZero()) { 
                break; 
            } 
            
            current_ray = Ray3f(x, frame.toWorld(bsdf_record.wo));
            found_intersection = scene->rayIntersect(current_ray, its);
            beta = beta * bsdf_term / (1-q);
            last_specular = !surface_bsdf->isDiffuse();
            bounces++;

            // Direct illumination with BRDF Importance Sampling
            // Done late as we need the collision result from the next step first
            if(found_intersection && find_emitter(its)) {
                
                const Emitter* emitter_hit = find_emitter(its);
                EmitterQueryRecord emitter_hit_record = EmitterQueryRecord(surface_bsdf, x, n, wi, its.p, its.shFrame.n, its.uv) ;

                Color3f emittance = emitter_hit->getEmittance(emitter_hit_record);

                float light_pdf = emitter_hit->pdf(emitter_hit_record, ESolidAngle);
                float brdf_pdf = surface_bsdf->pdf(bsdf_record);

                if(light_pdf == 0 || brdf_pdf == 0) {
                    continue;
                }

                float weight = balancedMIS(brdf_pdf, light_pdf) / brdf_pdf;
                Ld += direct(beta, weight, emittance);
            }
        }

        return Le + Ld;
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
        return "MISPathTracerIntegrator[]";
    }

private:
    float q;

};

NORI_REGISTER_CLASS(MISPathTracer, "path_mis");
NORI_NAMESPACE_END

