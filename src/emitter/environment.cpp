#include <core/common.h>
#include <emitter/emitter.h>
#include <emitter/environment.h>
#include <stats/warp.h>
#include <functional>

NORI_NAMESPACE_BEGIN

EnvironmentEmitter::EnvironmentEmitter(const PropertyList& props) :
    map1(props.getString("half1"), true), map2(props.getString("half2"), true) {

    radius = props.getFloat("radius");
    center = props.getPoint("center");
    height = props.getFloat("height");
}

float EnvironmentEmitter::pdf(const EmitterQueryRecord& rec) const {

    float pdf;

    if(is_on_map1(rec)) {
        pdf = weight_map1() * Warp::squareToGrayMapPdf(rec.uv, map1);
    }
    else {
        pdf = weight_map2() * Warp::squareToGrayMapPdf(rec.uv, map2);
    }

    return to_angular(rec, pdf);
}

Color3f EnvironmentEmitter::evalRadiance(EmitterQueryRecord& rec, const Scene* scene) const {

    // determinant of the jacobian of the change of coordinates
    float distortion_factor = angular_distortion(rec);

    Color3f emitted = getEmittance(rec);
    Color3f bsdf_term = evalBSDF(rec);

    return distortion_factor * (emitted * bsdf_term); 
}

Color3f EnvironmentEmitter::sampleRadiance(EmitterQueryRecord& rec, Sampler& sampler, const Scene* scene, float& angular_pdf) const {

    float pdf_light;
    samplePoint(sampler, rec, pdf_light);
    angular_pdf = to_angular(rec, pdf_light);

    if(!is_source_visible(scene, rec)) {
        return Color3f(0.f);
    }

    // determinant of the jacobian of the change of coordinates
    return evalRadiance(rec, scene) / pdf_light;
}

Color3f EnvironmentEmitter::getEmittance(const EmitterQueryRecord &rec) const {

    if(is_on_map1(rec)) {
        return map1.color(rec.uv);
    }
    else if(is_on_map2(rec)) {
        Point2f shifted_uv = rec.uv - Vector2f(map1.max_param());
        return map2.color(shifted_uv);
    }
    else {
        return Color3f(0.f);
    }
}

void EnvironmentEmitter::samplePoint(Sampler &sampler, EmitterQueryRecord &rec, float &pdf) const {
    float sample = sampler.next1D();

    if(sample < weight_map1()) {
        Vector3f n_l = (rec.l - center); n_l.y() = 0; n_l = n_l.normalized();
        rec.uv = Warp::squareToGrayMap(sampler.next2D(), map1);
        rec.l = map1_to_world(rec.uv);
        rec.n_l = n_l;
    } else {
        Vector3f n_l = (rec.l - center); n_l.y() = 0; n_l = n_l.normalized();
        rec.uv = Warp::squareToGrayMap(sampler.next2D(), map2);
        rec.l = map2_to_world(rec.uv);
        rec.n_l = n_l;
    }

    pdf = this->pdf(rec);
}

bool EnvironmentEmitter::is_on_map1(const EmitterQueryRecord &rec) const{
    float u = rec.uv.x();
    if(u < 0 || u > map1.max_param() + map2.max_param()) {
        return false;
    }
    return u <= map1.max_param();
}

bool EnvironmentEmitter::is_on_map2(const EmitterQueryRecord &rec) const{
    float u = rec.uv.x();
    if(u < 0 || u > map1.max_param() + map2.max_param()) {
        return false;
    }
    return u > map1.max_param();
}

Point2i EnvironmentEmitter::world_to_map1(const EmitterQueryRecord &rec) const{
    throw NoriException("This should not be needed");
    /*float ampl = radius;

    float x_norm = clamp(rec.x() / ampl, -1.f, 1.f);
    float theta = acos(x_norm);
    float x = theta / M_PI * (map1.max_param());
    float y = (rec.y() / height + 0.5f) * (map1.max_param());
    x = clamp(x, 0.f, (float) map1.max_param());
    y = clamp(y, 0.f, (float) map1.max_param());


    return Point2i((int) x, (int) map1.max_param() - (int) y);*/
}

Point2i EnvironmentEmitter::world_to_map2(const EmitterQueryRecord &rec) const{
    throw NoriException("This should not be needed");
    /*float ampl = radius;

    float x_norm = clamp(-rec.x() / ampl, -1.f, 1.f);
    float theta = acos(x_norm);
    float x = theta / M_PI * (map2.max_param());
    float y = (rec.y() / height + 0.5f) * (map2.max_param());

    x = clamp(x, 0.f, (float) map2.max_param());
    y = clamp(y, 0.f, (float) map2.max_param());


    return Point2i((int) x, (int) map2.max_param() - (int) y);*/
}

Point3f EnvironmentEmitter::map1_to_world(const Point2f &coords) const{

    float x_norm = coords.x() / (map1.max_param());
    float y_norm = 1 - coords.y() / (map1.max_param());

    float theta = M_PI * x_norm;
    float ampl = radius;
    float x = -ampl * cos(theta);
    float z = ampl * sin(theta);
    float y = 0.5f * height * (2*y_norm - 1);

    return center + Point3f(x, y, z);
}

Point3f EnvironmentEmitter::map2_to_world(const Point2f &coords) const{
    float x_norm = coords.x() / (map1.max_param());
    float y_norm = 1 - coords.y() / (map1.max_param());

    float angle = M_PI * x_norm;
    float ampl = radius;

    float x = ampl * cos(angle);
    float z = -ampl * sin(angle);
    float y = 0.5f * height * (2*y_norm - 1);

    return center + Point3f(x, y, z);
}

float EnvironmentEmitter::weight_map1() const{
    return map1.get_luminance() / (map1.get_luminance() + map2.get_luminance());
}



NORI_NAMESPACE_END
