#include <core/common.h>
#include <emitter/emitter.h>
#include <emitter/environment.h>
#include <stats/warp.h>
#include <functional>
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
}

float EnvironmentEmitter::pdf(const EmitterQueryRecord& rec) const {

    float pdf;

    if(is_on_map1(rec)) {
        Point2f remapped_uv = Point2f(2.f * rec.uv.x(), 1.f - rec.uv.y());
        pdf = weight_map1() * Warp::squareToGrayMapPdf(remapped_uv, map1);
    }
    else if(is_on_map2(rec)){
        Point2f remapped_uv = Point2f(2.f * (rec.uv.x() - 0.5f), 1.f - rec.uv.y());
        pdf = weight_map2() * Warp::squareToGrayMapPdf(remapped_uv, map2);
    } else {
        return 0.f;
    }

    // Warp::pdf assumes the surface is of length 1, which is not the case after spherical mapping
    float area = 4 * M_PI * radius * radius;

    return to_angular(rec, pdf / area);
}

Color3f EnvironmentEmitter::evalRadiance(const EmitterQueryRecord &rec, const Scene* scene) const {

    // determinant of the jacobian of the change of coordinates
    float distortion_factor = angular_distortion(rec);

    Color3f emitted = getEmittance(rec);
    Color3f bsdf_term = evalBSDF(rec);

    return intensity * distortion_factor * (emitted * bsdf_term);
}

Color3f EnvironmentEmitter::sampleRadiance(EmitterQueryRecord& rec, Sampler& sampler, const Scene* scene, float& angular_pdf) const {

    float pdf_light;
    samplePoint(sampler, rec, pdf_light);
    angular_pdf = to_angular(rec, pdf_light);

    if(!is_source_visible(scene, rec)) {
        return Color3f(0.f);
    }

    return evalRadiance(rec, scene) / pdf_light;
}

Color3f EnvironmentEmitter::getEmittance(const EmitterQueryRecord &rec) const {

    Point2f uv = is_on_map1(rec)
        ? Point2f(2.f * rec.uv.x(), rec.uv.y())
        : Point2f(2.f * (rec.uv.x() - 0.5f), rec.uv.y());

    const MipMap& map = is_on_map1(rec) ? map1 : map2;

    return map.color(uv, lerp);
}

void EnvironmentEmitter::samplePoint(Sampler &sampler, EmitterQueryRecord &rec, float &pdf) const {
    float sample = sampler.next1D();

    if(sample < weight_map1()) {
        Point2f uv = Warp::squareToGrayMap(sampler.next2D(), map1);
        uv.x() *= 0.5f;
        rec.uv = uv;
        rec.l = map1_to_world(rec.uv);

        Vector3f n_l = (center - rec.l); n_l.y() = 0; n_l = n_l.normalized();
        rec.n_l = n_l;
    } else {
        Point2f uv = Warp::squareToGrayMap(sampler.next2D(), map2);
        uv.x() = 0.5f*uv.x() + 0.5f;
        rec.uv = uv;
        rec.l = map2_to_world(rec.uv);
        Vector3f n_l = (center - rec.l); n_l.y() = 0; n_l = n_l.normalized();
        rec.n_l = n_l;
    }

    pdf = this->pdf(rec);
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
    /*float theta = M_PI * x_norm;
    float ampl = radius;
    float x = -ampl * cos(theta);
    float z = ampl * sin(theta);
    float y = 0.5f * height * (2*y_norm - 1);

    return center + Point3f(x, y, z);*/
    return center + radius * sphericalDirection(theta, phi);
}

Point3f EnvironmentEmitter::map2_to_world(const Point2f &coords) const{
    float x_norm = coords.x();
    float y_norm = 1 - coords.y();

    float theta = y_norm * M_PI;
    float phi = x_norm * 2 * M_PI;
    /*
    float angle = M_PI * x_norm;
    float ampl = radius;

    float x = ampl * cos(angle);
    float z = -ampl * sin(angle);
    float y = 0.5f * height * (2*y_norm - 1);

    return center + Point3f(x, y, z);*/
    return center + radius * sphericalDirection(theta, phi);
}

float EnvironmentEmitter::weight_map1() const{
    return map1.get_luminance() / (map1.get_luminance() + map2.get_luminance());
}

NORI_NAMESPACE_END
