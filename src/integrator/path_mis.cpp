#include <integrator/integrator.h>
#include <emitter/emitter.h>
#include <core/scene.h>
#include <bsdf/bsdf.h>
#include <stats/warp.h>

NORI_NAMESPACE_BEGIN

class MisPathTracer : public Integrator {
public:
    MisPathTracer(const PropertyList &props) {

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

        int min_bounces = 3;
        float eta = 1.0f;
        float q = 1.0;

        while(bounces <= min_bounces || sampler->next1D() < q) {

            if(!found_intersection) {
                break;
            }

            beta /= q;

            const Point3f x = its.p;
            const Vector3f n = its.shFrame.n;
            const Vector3f wi = -current_ray.d;
            const Emitter* hit_emitter = its.mesh->getEmitter();
            const BSDF* surface_bsdf = its.mesh->getBSDF();
            Frame frame(n);
            EmitterQueryRecord emitter_rec = EmitterQueryRecord(surface_bsdf, x, n, wi, its.uv);


            if(hit_emitter) {
                if(bounces == 0 || last_specular) {
                    EmitterQueryRecord hit_emitter_rec = EmitterQueryRecord(nullptr, current_ray.o, 0.f, current_ray.d, x, n, its.uv);
                    Color3f emittance = hit_emitter->getEmittance(hit_emitter_rec);
                    Le += beta * emittance;
                }
                break;
            }

            // Direct Illumination using Emitter Importance Sampling
            float emitter_pdf;
            Emitter* emitter = scene->pickUniformEmitter(emitter_pdf);

            float pdf_light;
            float pdf_light_angular;

            Color3f direct_rad = emitter->sampleRadiance(emitter_rec, *sampler, scene, pdf_light, EMeasure::ESurfaceArea);
            pdf_light *= emitter_pdf;
            direct_rad /= pdf_light;

            if(!direct_rad.isZero()) {
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
            beta *= bsdf_term;
            eta *= bsdf_record.eta;
            last_specular = !surface_bsdf->isDiffuse();
            bounces++;

            // Direct illumination with BRDF Importance Sampling
            // Done late as we need the collision result from the next step first
            if(found_intersection && its.mesh->getEmitter()) {

                const Emitter* emitter_hit = its.mesh->getEmitter();
                EmitterQueryRecord emitter_hit_record = EmitterQueryRecord(surface_bsdf, x, n, wi, its.p, its.shFrame.n, its.uv) ;

                Color3f emittance = emitter_hit->getEmittance(emitter_hit_record);

                float light_pdf = emitter_hit->pdf(emitter_hit_record, ESolidAngle);
                light_pdf *= emitter_pdf;
                float brdf_pdf = surface_bsdf->pdf(bsdf_record);

                if(light_pdf != 0 || brdf_pdf != 0) {
                    float late_weight = balancedMIS(brdf_pdf, light_pdf);
                    Ld += direct(beta, late_weight, emittance);
                }
            }

            // do not give the "bonus" for surviving russian roulette if we're not using it yet
            q = bounces <= min_bounces
                ? 1.0f
                : 0.9f;

        }
        return Le + Ld;
    }

    std::string toString() const {
        return "MisIntegrator[]";
    }

    static float linf(const Color3f& c) {
        return std::max({c.x(), c.y(), c.z()});
    }
};

NORI_REGISTER_CLASS(MisPathTracer, "path_mis");
NORI_NAMESPACE_END