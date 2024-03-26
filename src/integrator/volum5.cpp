#include <integrator/integrator.h>
#include <core/scene.h>
#include <stats/warp.h>
#include <volume/medium.h>

NORI_NAMESPACE_BEGIN

/// Volumetric integrator. Supports:
/// - Homogeneous medium absorption and scattering
/// - NEE + Indirect scattering (no MIS)
/// - Both emitter and regular surfaces
class Volum5Integrator : public Integrator {
public:
    explicit Volum5Integrator(const PropertyList &props) {
        /* No parameters this time */
    }

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {

        float q = 0.1;
        int max_bounces = 10;

        Color3f Le(0.f), Li(0.f);
        Color3f beta(1.f);
        Ray3f current_ray = ray;

        int bounces = 0; bool last_specular = false;

        while(sampler->next1D() > q && bounces <= max_bounces) {

            // short circuit to avoid computation if the contribution is zero
            if(beta.isZero()) {
                break;
            }

            Intersection its; bool found = scene->rayIntersect(current_ray, its);
            Vector3f wi = -current_ray.d;
            const Emitter* hit_emitter = find_emitter(its);

            if(hit_emitter) {
                if(!its.medium.is_present() && (bounces == 0 || last_specular)) {
                    Le += beta * emittance(its);
                }

                if (bounces != 0) {
                    break;
                }
            }

            bool scatters;
            Point3f intersection_point;
            float tr_over_pdf;
            float light_point_pdf;
            bool valid_ray = sampleIntersection(found, current_ray, its, sampler, scatters, tr_over_pdf, intersection_point);

            // If there is no medium AND no intersection is found, there is nothing else to do
            if(!valid_ray) {
                break;
            }
            // NEE for either medium scattering or surface intersection
            for(const Emitter* emitter : scene->getEmitters()) {

                EmitterQueryRecord record = recordForNEE(current_ray, its, intersection_point, scatters);
                emitter->samplePoint(*sampler, record, light_point_pdf, EMeasure::ESolidAngle);

                Color3f directional_term = directionalChangeTerm(scatters, its, record);
                Color3f emitted = emitter->getEmittance(record);

                if(light_point_pdf != 0.f && emitter->is_source_visible(scene, record)) {
                    // transmittance/pdf = 1
                    Li += tr_over_pdf * beta * emitted * directional_term / light_point_pdf;
                }
            }

            // Handle indirect lighting
            Vector3f next_direction = sampleNextDirection(sampler, current_ray.d, scatters, its, beta);
            current_ray = Ray3f(intersection_point, next_direction);
            current_ray.starting_medium = its.medium.medium; // potentially nullptr (and it's fine)
            last_specular = !scatters && !its.mesh->getBSDF()->isDiffuse(); // if no scattering, the BSDF must exist
            bounces++;
            beta /= (1-q);
        }
        return Le + Li;
    }

    static Color3f directionalChangeTerm(bool scattering, const Intersection& its, const EmitterQueryRecord& rec) {

        if(scattering) {
            float omega_s = its.medium.medium->out_scattering(0.f, 0.f);
            return omega_s * its.medium.medium->evalPhase(rec.wi, rec.wo());
        } else {
            BSDFQueryRecord bsdf_query(rec.wi, rec.wo(), its.shFrame, EMeasure::ESolidAngle, its.uv);
            return its.mesh->getBSDF()->eval(bsdf_query);
        }
    }

    static Vector3f sampleNextDirection(Sampler* sampler, const Vector3f& in, bool scattering, const Intersection& its, Color3f& beta) {

        Vector3f next; float unused;

        if(scattering) {
            const Medium *m = its.medium.medium;
            m->samplePhase(sampler, in, next, unused);
            beta *= m->out_scattering(0.f, 0.f) * m->evalPhase(in, next);
        } else {
            const BSDF* bsdf = its.mesh->getBSDF();
            BSDFQueryRecord record(its.shFrame.toLocal(-in));
            beta *= bsdf->sample(record, sampler->next2D());
            next = its.shFrame.toWorld(record.wo);
        }

        return next;
    }

    static EmitterQueryRecord recordForNEE(const Ray3f& ray, const Intersection& its, const Point3f& p, bool scattering) {
        if(scattering) {
            return EmitterQueryRecord(nullptr, p, 0.f, -ray.d, 0.f);
        } else {
            return EmitterQueryRecord(nullptr, p, its.shFrame.n, -ray.d, its.uv);
        }
    }

    static bool sampleIntersection(bool found, const Ray3f& ray, const Intersection& its, Sampler* sampler, bool& scattering, float& tr_pdf_ratio, Point3f& intersection) {

        if(!its.medium.is_present() && !found) {
            return false;
        }

        // if no medium, PDF is a discrete dirac delta
        if(!its.medium.is_present()) {
            scattering = false;
            tr_pdf_ratio = 1.f;
            intersection = its.p;
            return true;
        }

        float omega_t = its.medium.medium->attenuation(0.f, 0.f);
        float t_max = maxt(found, its);
        float t_through_media = Warp::lineToHomogeneousPath(sampler->next1D(), omega_t);
        float t_travelled = its.medium.mint + t_through_media;

        // Medium found, but the ray still travelled all the way through it
        if(t_travelled >= t_max) {
            if(!found) {
                // no surface hit, and the ray went through the medium
                return false;
            }
            scattering = false;
            tr_pdf_ratio = 1.f; // discrete PDF
            intersection = its.p;
            return true;
        }

        // Medium found, and scattering happened
        intersection = ray.o + t_travelled * ray.d;
        scattering = true;
        tr_pdf_ratio = 1.f / omega_t;
        return true;
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

    static float maxt(bool found, const Intersection& its) {
        return found ? its.t : std::numeric_limits<float>::infinity();
    }

    static Color3f emittance(const Intersection& its) {
        const Emitter* emitter = its.emitter ? its.emitter : its.mesh->getEmitter();
        if(!emitter) {
            throw NoriException("Can't find emittance as the intersection is not an emitter");
        }
        return emitter->getEmittance(EmitterQueryRecord(nullptr, 0.f, 0.f, 0.f, its.p, its.shFrame.n, its.uv));
    }

    std::string toString() const {
        return "Volum5Integrator[]";
    }
};

NORI_REGISTER_CLASS(Volum5Integrator, "volum5");
NORI_NAMESPACE_END

