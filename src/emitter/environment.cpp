#include <core/common.h>
#include <emitter/emitter.h>
#include <emitter/environment.h>
#include <stats/warp.h>
#include <functional>

NORI_NAMESPACE_BEGIN

EnvironmentEmitter::EnvironmentEmitter(const PropertyList& props) :
    map1(props.getString("half1"), false), map2(props.getString("half2"), false) {

    radius = props.getFloat("radius");
    center = props.getPoint("center");
    height = props.getFloat("height");
}

float EnvironmentEmitter::pdf(const EmitterQueryRecord& rec) const {
    
    float d2 = (rec.l - rec.p).squaredNorm();
    float cos_theta = rec.n_l.dot((rec.p - rec.l).normalized());

    float pdf;

    if(is_on_map1(rec.l)) {
        Point2i to_map1 = world_to_map1(rec.l);
        pdf = weight_map1() * Warp::squareToGrayMapPdf(Point2f((float) to_map1.x(), (float) to_map1.y()), map1);
    } else {
        Point2i to_map2 = world_to_map2(rec.l);
        pdf = weight_map2() * Warp::squareToGrayMapPdf(Point2f((float) to_map2.x(), (float) to_map2.y()), map2);
    }

    return pdf * d2 / cos_theta;
}

Color3f EnvironmentEmitter::evalRadiance(EmitterQueryRecord& rec, const Scene* scene) const {
    
    Vector3f x_to_y = (rec.l - rec.p).normalized();
    float distance = (rec.l - rec.p).norm();

    // determinant of the jacobian of the change of coordinates
    float distortion_factor = abs(rec.n_p.dot(x_to_y) * (rec.n_l.dot(x_to_y))) / (distance*distance);

    Color3f emitted = getEmittance(rec.l, rec.n_l, -x_to_y);

    Frame frame(rec.n_p); // BSDFQueryRecord expects local vectors
    BSDFQueryRecord query(frame.toLocal(rec.wi), frame.toLocal(x_to_y), EMeasure::ESolidAngle);

    Color3f bsdf_term = rec.bsdf->eval(query);

    return distortion_factor * (emitted * bsdf_term); 
}

Color3f EnvironmentEmitter::sampleRadiance(EmitterQueryRecord& rec, Sampler& sampler, const Scene* scene, float& angular_pdf) const {
    
    float sample = sampler.next1D();

    Point2f uv;
    Point3f l;
    float pdf_light;

    if(sample < weight_map1()) {
        uv = Warp::squareToGrayMap(sampler.next2D(), map1);
        l = map1_to_world(uv);
        pdf_light = Warp::squareToGrayMapPdf(uv, map1);
    } else {
        uv = Warp::squareToGrayMap(sampler.next2D(), map2);
        l = map2_to_world(uv);
        pdf_light = Warp::squareToGrayMapPdf(uv, map2);
    }
  
    Vector3f n_l = (l - center); n_l.y() = 0;
    Vector3f x_to_y = (l - rec.p).normalized();
    float distance = (l - rec.p).norm();
    Color3f emitted = getEmittance(l, n_l, -x_to_y);


    // We stop the ray right before its intersection with the light source (which would be guaranteed to happen)
    Ray3f ray = Ray3f(rec.p, x_to_y, Epsilon, (1 - Epsilon) * distance);

    if(scene->rayIntersect(ray)) {
        // meaning the ray hit an object BEFORE hitting the light source
        return Color3f(0.0f);
    }

    // determinant of the jacobian of the change of coordinates
    float distortion_factor = abs(rec.n_p.dot(x_to_y) * (n_l.dot(x_to_y))) / (distance*distance);

    // Evaluate BSDF term
    Frame frame(rec.n_p); // BSDFQueryRecord expects local vectors
    BSDFQueryRecord query(frame.toLocal(rec.wi), frame.toLocal(x_to_y), EMeasure::ESolidAngle, rec.uv);
    Color3f bsdf_term = rec.bsdf->eval(query);

    // Provide angular version of PDF
    angular_pdf = pdf_light * distance * distance / rec.n_p.dot(x_to_y);

    // Set record parameters + return output color
    rec.l = l;
    rec.n_l = n_l;
    return distortion_factor * (emitted * bsdf_term) / pdf_light; 

    
    /*float sample = sampler.next1D();
    float w1 = map1.get_luminance() / (map1.get_luminance() + map2.get_luminance());

    const MipMap& map = (sample < w1) ? map1 : map2;
    float factor = (sample < w1) ? 1.f : -1.f;

    Point2f point = Warp::squareToGrayMap(sampler.next2D(), map);

    Color3f color = map.color((int) point.x(), (int) point.y());

    float angle = factor * M_PI * point.x() / (map.max_resolution() - 1);
    float height = height * (2*point.y()/(map.max_resolution() - 1) - 1);

    float dist = radius.norm();
    Point3f l = center + Point3f(dist*cos(angle), height, dist*sin(angle));
    Vector3f n = (l - (center + Point3f(0.f, height, 0.f))).normalized();

    rec.l = l;
    rec.n_l = l;
    float pdf = Warp::squareToGrayMapPdf(point, map);
    return color / pdf;*/
}

Color3f EnvironmentEmitter::getEmittance(Point3f pos, Vector3f normal, Vector3f direction) const {
    if(is_on_map1(pos)) {
        Point2i p = world_to_map1(pos);
        return map1.color(p);
    } else if(is_on_map2(pos)) {
        Point2i p = world_to_map2(pos);
        return map2.color(p);
    } else {
        return 0.f;
    }
}

bool EnvironmentEmitter::is_on_map1(const Point3f &p) const{
    return p.z() > 0;
}

bool EnvironmentEmitter::is_on_map2(const Point3f &p) const{
    return p.z() <= 0;
}

Point2i EnvironmentEmitter::world_to_map1(const Point3f &p) const{
    float ampl = radius;

    float theta = acos(std::max(-1.f,-p.x() / ampl));
    float x = theta / M_PI * (map1.max_resolution() - 1);
    float y = (p.y() / height + 0.5f) * (map1.max_resolution() - 1);
    x = clamp(x, 0.f, (float) map1.max_resolution() - 1);
    y = clamp(y, 0.f, (float) map1.max_resolution() - 1);


    return Point2i((int) x, (map1.max_resolution() - 1) - (int) y);
}

Point2i EnvironmentEmitter::world_to_map2(const Point3f &p) const{
    float ampl = radius;

    float theta = acos(std::min(1.f,p.x() / ampl));
    float x = theta / M_PI * (map2.max_resolution() - 1);
    float y = (p.y() / height + 0.5f) * (map2.max_resolution() - 1);

    x = clamp(x, 0.f, (float) map2.max_resolution() - 1);
    y = clamp(y, 0.f, (float) map2.max_resolution() - 1);


    return Point2i((int) x, (map2.max_resolution() - 1) - (int) y);
}

Point3f EnvironmentEmitter::map1_to_world(const Point2f &coords) const{

    float x_norm = coords.x() / (map1.max_resolution() - 1);
    float y_norm = 1 - coords.y() / (map1.max_resolution() - 1);

    float theta = M_PI * x_norm;
    float ampl = radius;
    float x = -ampl * cos(theta);
    float z = ampl * sin(theta);
    float y = 0.5f * height * (2*y_norm - 1);

    return center + Point3f(x, y, z);
}

Point3f EnvironmentEmitter::map2_to_world(const Point2f &coords) const{
    float x_norm = coords.x() / (map1.max_resolution() - 1);
    float y_norm = 1 - coords.y() / (map1.max_resolution() - 1);

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
