#include <integrator/integrator.h>
#include <core/scene.h>
#include <stats/warp.h>
#include <volume/medium.h>

NORI_NAMESPACE_BEGIN

/// Volumetric integrator. Supports:
/// - Heterogeneous medium absorption and scattering
/// - NEE + Indirect scattering with MIS
/// - Both emitter and regular surfaces
class Volum7Integrator : public Integrator {
public:
    explicit Volum7Integrator(const PropertyList &props) {
        /* No parameters this time */
    }

    constexpr static float Q = 0.1f;
    constexpr static int MAX_BOUNCES = 10;

    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {

        Color3f Le(0.f), Li(0.f);
        Color3f beta(1.f);
        Ray3f current_ray = ray;

        int bounces = 0; bool last_specular = false;
        Intersection its;
        bool found;

        found = scene->rayIntersect(current_ray, its);

        while(roulette_success(sampler, bounces)) {

            beta /= (1-Q);

            // short circuit to avoid computation if the contribution is zero
            if(beta.isZero()) {
                break;
            }

            Vector3f wi = -current_ray.d;
            const Emitter* hit_emitter = find_emitter(its);
            const BSDF* hit_bsdf = its.mesh ? its.mesh->getBSDF() : nullptr;
            const Vector3f& surface_normal = found ? its.shFrame.n : 0.f;

            bool scatters;
            Point3f intersection_point;
            float tr_over_pdf;

            bool any_intersection = sampleIntersection(found, current_ray, its, sampler,
                                                       scatters, tr_over_pdf, intersection_point);

            // If there is no medium AND no intersection is found, there is nothing else to do
            if(!any_intersection) {
                break;
            }

            beta *= tr_over_pdf;

            if(!scatters && hit_emitter && (bounces == 0 || last_specular)) {
                add_illumination(Le, beta * emittance(its));
            }

            // If no BSDF we hit an object that can't continue the path tracing algorithm
            // such as an environment map
            if(hit_emitter && !its.mesh && !scatters) {
                break;
            }

            // NEE for either medium scattering or surface intersection with Emitter Importance Sampling
            for(const Emitter* emitter : scene->getEmitters()) {

                float light_point_pdf;
                // Sample point on the emitter
                EmitterQueryRecord record = recordForNEE(current_ray, its, intersection_point, scatters);
                emitter->samplePoint(*sampler, record, light_point_pdf, EMeasure::ESurfaceArea);

                // Sample emittance
                Color3f emitted = emitter->getEmittance(record) / light_point_pdf;

                // Cost induced by either the BRDF, or the Phase function
                Color3f directional_cost = directionalChangeTerm(scatters, its, record);

                float angular_distortion = !scatters
                        ? emitter->angular_distortion(record)
                        : 1 / emitter->to_angular(record, 1.f);

                /*
                 * Trace ray between "intersection_point" and "record.l"
                 * We need to know if there is a medium, and it's not necessarily the same medium
                 * This is a bit annoying as we'd rather just perform a shadow ray, but we need that medium information
                 */
                bool obstructed;
                Intersection direct_its;
                Ray3f direct_ray = Ray3f(intersection_point, record.wo(), Epsilon, (1-Epsilon) * record.dist());
                if(its.medium.is_present() && its.medium.medium->bounds().contains(intersection_point)) {
                    direct_ray.starting_medium = its.medium.medium;
                }
                obstructed = scene->rayIntersect(direct_ray, direct_its);

                if(obstructed) {
                    continue;
                }

                // Transmittance of NEE. Potentially 1.0 if there is no media blocking the path
                float direct_transmittance = eval_transmittance(direct_its.medium.medium, record, sampler);

                // evaluate hypothetical phase function / BRDF pdf for MIS
                float light_point_angular_pdf = emitter->to_angular(record, light_point_pdf);
                float directional_pdf  = directional_change_pdf(scatters, its, record);

                if(directional_pdf == 0 || light_point_angular_pdf == 0) {
                    continue;
                }

                float weight = 1.f;//balancedMIS(light_point_angular_pdf, directional_pdf);

                if(emitter->is_source_visible(scene, record)) {
                    add_illumination(Li, weight * direct_transmittance * beta * emitted * directional_cost * angular_distortion);
                }
            }

            // Handle indirect lighting
            Ray3f previous_ray = current_ray;
            Intersection previous_its = its;

            // sample next direction and include loss of energy due to scattering
            Vector3f next_direction = sampleNextDirection(sampler, current_ray.d, scatters, its, beta);
            current_ray = Ray3f(intersection_point, next_direction);

            if(its.medium.is_present()) {
                if(its.medium.medium->bounds().contains(intersection_point)) {
                    current_ray.starting_medium = its.medium.medium;
                } else {
                    current_ray.starting_medium = nullptr;
                }
            }

            last_specular = !scatters && !its.mesh->getBSDF()->isDiffuse(); // if no scattering, the BSDF must exist
            bounces++;
            its.medium = MediumInteraction();
            found = scene->rayIntersect(current_ray, its);


            // Direct illumination with BRDF Importance Sampling
            // Done late as we need the collision result from the next step first
            if(false && found && find_emitter(its)) {

                const Emitter* emitter_hit = find_emitter(its);
                EmitterQueryRecord emitter_hit_record = recordForLateNEE(previous_ray,
                                                                         its,
                                                                         hit_bsdf,
                                                                         intersection_point,
                                                                         surface_normal,
                                                                         scatters);

                Color3f emitted = emitter_hit->getEmittance(emitter_hit_record);
                Color3f directional_term = directionalChangeTerm(scatters, previous_its, emitter_hit_record);
                float direct_transmittance = eval_transmittance(its.medium.medium, emitter_hit_record, sampler);

                if(emitted.isZero()) {
                    continue;
                }

                float light_pdf = emitter_hit->pdf(emitter_hit_record, ESolidAngle);
                float brdf_pdf = directionalChangePdf(scatters, previous_its, -previous_ray.d, next_direction);

                if(light_pdf + brdf_pdf == 0) {
                    continue;
                }

                float previous_tr_over = its.medium.is_present()
                        ? its.medium.medium->attenuation(intersection_point, 0.f)
                        : 1.0f;

                float weight = balancedMIS(brdf_pdf, light_pdf);
                add_illumination(Li,
                                 weight * direct_transmittance * beta * emitted * directional_term);
            }
        }

        return Le + Li;
    }

    static Color3f directionalChangeTerm(bool scattering, const Intersection& its, const EmitterQueryRecord& rec) {

        if(scattering) {
            float omega_s = its.medium.medium->out_scattering(rec.p, 0.f);
            return omega_s * its.medium.medium->evalPhase(rec.wi, rec.wo());
        } else {
            BSDFQueryRecord bsdf_query(rec.wi, rec.wo(), its.shFrame, EMeasure::ESolidAngle, its.uv);
            return its.mesh->getBSDF()->eval(bsdf_query);
        }
    }

    static float directional_change_pdf(bool scattering, const Intersection& its, const EmitterQueryRecord& rec) {
        if(scattering) {
            return its.medium.medium->phasePdf(-rec.wi, rec.wo());
        } else {
            BSDFQueryRecord bsdf_query(rec.wi, rec.wo(), its.shFrame, EMeasure::ESolidAngle, its.uv);
            return its.mesh->getBSDF()->pdf(bsdf_query);
        }
    }

    static float directionalChangePdf(bool scattering, const Intersection& its, const Vector3f& wi, const Vector3f& wo) {
        if(scattering) {
            return its.medium.medium->phasePdf(-wi, wo);
        } else {
            BSDFQueryRecord bsdf_query(wi, wo, its.shFrame, EMeasure::ESolidAngle, its.uv);
            return its.mesh->getBSDF()->pdf(bsdf_query);
        }
    }

    static Vector3f sampleNextDirection(Sampler* sampler, const Vector3f& in, bool scattering, const Intersection& its, Color3f& beta) {

        Vector3f next; float unused;

        if(scattering) {
            const Medium *m = its.medium.medium;
            m->samplePhase(sampler, in, next, unused);
            beta *= m->out_scattering(its.p, 0.f) * m->evalPhase(in, next);
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
            return EmitterQueryRecord(find_bsdf(its), p, its.shFrame.n, -ray.d, its.uv);
        }
    }

    static EmitterQueryRecord recordForLateNEE(
            const Ray3f& previous_ray, const Intersection& new_its, const BSDF* previous_bsdf, const Point3f& previous_p, const Vector3f& previous_n, bool scattering) {

        if(scattering) {
            return EmitterQueryRecord(
                    nullptr, previous_p, 0.f, -previous_ray.d,
                    new_its.p, new_its.shFrame.n, new_its.uv
                    );
        } else {
            return EmitterQueryRecord(
                    previous_bsdf, previous_p, previous_n, -previous_ray.d,
                    new_its.p, new_its.shFrame.n, new_its.uv);
        }
    }

    static bool sampleIntersection(bool contains_surface, const Ray3f& ray, const Intersection& its, Sampler* sampler,
                                   bool& scattering, float& tr_over_pdf, Point3f& intersection) {

        const MediumInteraction& mits = its.medium;

        if(!mits.is_present() && !contains_surface) {
            return false;
        }

        // if no medium, PDF is a discrete dirac delta
        // Transmittance is 1, pdf is 1 (as only one possibility)
        if(!mits.is_present()) {
            scattering = false;
            tr_over_pdf = 1.f;
            intersection = its.p;
            return true;
        }

        Point3f medium_entrance = ray.o + mits.mint * ray.d;
        Point3f medium_exit = ray.o + mits.maxt * ray.d;

        float t_max = maxt(contains_surface, its);

        // Very conveniently, the warping scheme samples as though the medium was of infinite volume,
        // but any point outside its defined spectrum has infinite density. This stopping condition allows us to
        // still define the normalization pdf factor as omega_t(stopping point), as P(tmax > t) = p(tmax), as the pdf
        // becomes a dirac delta on the edges
        float omega_t;
        float t_through_media = Warp::sampleHeterogeneousDistance(sampler, medium_entrance, ray.d, *mits.medium, omega_t);
        float t_travelled = mits.mint + t_through_media;


        // Medium found, but the ray still travelled all the way through it
        // Or, meaning the delta tracking didn't encounter a single non-zero value
        // Or, that the medium traversal had such a narrow window that it skipped it (and it's fine)
        // TODO: Note: for now it doesn't work if a surface is inside a media - doesnt it?
        if(omega_t < 0 || t_travelled > t_max) {
            if(!contains_surface) {
                // no surface hit, and the ray went through the medium
                return false;
            }
            scattering = false;
            tr_over_pdf = 1.0f;
            intersection = its.p;
            return true;
        }

        // Medium contains_surface, and scattering happened
        intersection = ray.o + t_travelled * ray.d;
        tr_over_pdf = 1.0f / omega_t;
        scattering = true;

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
        float v = std::numeric_limits<float>::infinity();
        if(found) v = std::min(v, its.t);
        if(its.medium.is_present()) v = std::min(v, its.medium.maxt);

        return v;
    }

    static Color3f emittance(const Intersection& its) {
        const Emitter* emitter = its.emitter ? its.emitter : its.mesh->getEmitter();
        if(!emitter) {
            throw NoriException("Can't find emittance as the intersection is not an emitter");
        }
        return emitter->getEmittance(EmitterQueryRecord(nullptr, 0.f, 0.f, 0.f, its.p, its.shFrame.n, its.uv));
    }

    static const BSDF* find_bsdf(const Intersection& its) {
        return its.mesh ? its.mesh->getBSDF() : nullptr;
    }

    static void add_illumination(Color3f& src, const Color3f& val) {
        if(!val.isValid()) {
            throw NoriException("Invalid radiance");
        }
        src += val;
    }

    static bool roulette_success(Sampler* sampler, int bounces) {
        return sampler->next1D() > Q && bounces <= MAX_BOUNCES;
    }

    static float eval_transmittance(const Medium* medium, const EmitterQueryRecord& record, Sampler* sampler) {
        if(!medium) {
            return 1.0f;
        }
        return Warp::ratio_tracking(record.p, record.l, *medium, sampler);
    }

    std::string toString() const {
        return "Volum7Integrator[]";
    }
};

NORI_REGISTER_CLASS(Volum7Integrator, "volum7");
NORI_NAMESPACE_END

