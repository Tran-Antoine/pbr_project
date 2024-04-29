#include <core/common.h>
#include <emitter/emitter.h>
#include <emitter/environment.h>
#include <stats/warp.h>
#include <core/color.h>

NORI_NAMESPACE_BEGIN

EnvironmentEmitter::EnvironmentEmitter(const PropertyList& props) :
    map1(props.getString("half1"), true), map2(props.getString("half2"), true),
    Sphere(props.getFloat("radius")){

    radius = props.getFloat("radius");
    center = props.getPoint("center");
    height = props.getFloat("height", 0.f);
    intensity = props.getFloat("intensity");
    lerp = props.getBoolean("lerp-transition", false);
    theta0 = props.getFloat("theta0", 0.f);
    phi0 = props.getFloat("phi0", 0.f);
}

float EnvironmentEmitter::pdf(const EmitterQueryRecord &rec, EMeasure unit) const {

    float pdf = 0.f;
    float map_area_2;

    if(is_on_map1(rec)) {
        Point2f remapped_uv = Point2f(2.f * rec.uv.x(), rec.uv.y());
        map_area_2 = map1.max_resolution() * map1.max_resolution();
        pdf = weight_map1() * Warp::squareToGrayMapPdf(remapped_uv, map1);
    }
    else if(is_on_map2(rec)){
        Point2f remapped_uv = Point2f(2.f * (rec.uv.x() - 0.5f), rec.uv.y());
        map_area_2 = map2.max_resolution() * map2.max_resolution();
        pdf = weight_map2() * Warp::squareToGrayMapPdf(remapped_uv, map2);
    } else {
        return pdf;
    }

    // Warp::pdf assumes the surface is of length 1, which is not the case after spherical mapping
    // Hence we divide by the area of a half sphere
    pdf /= (2 * M_PI * radius * radius / map_area_2);

    switch(unit) {
        case EMeasure::ESurfaceArea: return pdf;
        case EMeasure::ESolidAngle: return to_angular(rec, pdf);
        default: throw NoriException("Unsupported unit");
    }
}

Color3f EnvironmentEmitter::evalRadiance(const EmitterQueryRecord &rec, const Scene* scene) const {

    // determinant of the jacobian of the change of coordinates
    float distortion_factor = angular_distortion(rec);

    Color3f emitted = getEmittance(rec);
    Color3f bsdf_term = evalBSDF(rec);

    return distortion_factor * (emitted * bsdf_term);
}

Color3f EnvironmentEmitter::sampleRadiance(EmitterQueryRecord &rec, Sampler &sampler, const Scene *scene, float &pdf,
                                   EMeasure unit) const {

    samplePoint(sampler, rec, pdf, unit);

    if(pdf == 0 || !is_source_visible(scene, rec)) {
        return Color3f(0.f);
    }

    return evalRadiance(rec, scene);
}

Color3f EnvironmentEmitter::getEmittance(const EmitterQueryRecord &rec) const {

    if(rec.n_l.dot(-rec.wo()) < 0) {
        return Color3f(0.0f);
    }

    Point2f uv = is_on_map1(rec)
        ? Point2f(2.f * rec.uv.x(), rec.uv.y())
        : Point2f(2.f * (rec.uv.x() - 0.5f), rec.uv.y());

    const MipMap& map = is_on_map1(rec) ? map1 : map2;

    return map.color(uv, lerp);
}

void EnvironmentEmitter::samplePoint(Sampler &sampler, EmitterQueryRecord &rec, float &pdf, EMeasure unit) const {
    float sample = sampler.next1D();

    if(sample < weight_map1()) {
        Point2f uv = Warp::squareToGrayMap(sampler.next2D(), map1);
        uv.x() *= 0.5f;
        rec.uv = uv;
        rec.l = map1_to_world(rec.uv);
        Vector3f n_l = (center - rec.l).normalized();;
        rec.n_l = n_l;
    } else {
        Point2f uv = Warp::squareToGrayMap(sampler.next2D(), map2);
        uv.x() = 0.5f*uv.x() + 0.5f;
        rec.uv = uv;
        rec.l = map2_to_world(rec.uv);
        Vector3f n_l = (center - rec.l).normalized();
        rec.n_l = n_l;
    }

    pdf = this->pdf(rec, unit);
}

bool EnvironmentEmitter::is_on_map1(const EmitterQueryRecord &rec) const{
    float u = rec.uv.x();
    if(u < 0.f || u > 1.f) {
        return false;
    }
    return u < 0.5f;
}

bool EnvironmentEmitter::is_on_map2(const EmitterQueryRecord &rec) const{
    float u = rec.uv.x();
    if(u < 0.f || u > 1.f) {
        return false;
    }
    return u >= 0.5f;
}

Point3f EnvironmentEmitter::map1_to_world(const Point2f &coords) const{

    float x_norm = coords.x();
    float y_norm = 1 - coords.y();

    float theta = y_norm * M_PI;
    float phi = x_norm * 2 * M_PI;

    float x = -radius * sin(theta) * cos(phi);
    float y = -radius * cos(theta);
    float z = -radius * sin(theta) * sin(phi);

    return center + Point3f(x, y, z);
}

Point3f EnvironmentEmitter::map2_to_world(const Point2f &coords) const{

    float x_norm = coords.x();
    float y_norm = 1 - coords.y();

    float theta = y_norm * M_PI;
    float phi = x_norm * 2 * M_PI;

    float x = -radius * sin(theta + theta0) * cos(phi + phi0);
    float y = -radius * cos(theta + theta0);
    float z = -radius * sin(theta + theta0) * sin(phi + phi0);
    return center + Point3f(x, y, z);
}

float EnvironmentEmitter::weight_map1() const{
    return map1.get_luminance() / (map1.get_luminance() + map2.get_luminance());
}

NORI_NAMESPACE_END
