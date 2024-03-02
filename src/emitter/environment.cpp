#include <core/common.h>
#include <emitter/emitter.h>
#include <emitter/environment.h>
#include <stats/warp.h>

NORI_NAMESPACE_BEGIN


EnvironmentEmitter::EnvironmentEmitter(const PropertyList& props) :
    map1(props.getString("half1"), false), map2(props.getString("half2"), false) {

    radius = props.getPoint("radius");
    center = props.getPoint("center");
    height = props.getFloat("height");
}

float EnvironmentEmitter::pdf(const EmitterQueryRecord& rec) const {
    
    float d2 = (rec.l - rec.p).squaredNorm();
    float cos_theta = rec.n_l.dot((rec.p - rec.l).normalized());

    float pdf;

    if(is_on_map1(rec.l)) {
        pdf = weight_map1() * Warp::squareToGrayMapPdf(rec.l, map1);
    } else {
        pdf = weight_map2() * Warp::squareToGrayMapPdf(rec.l, map2);
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
        return map1.color(world_to_map1(pos));
    } else if(is_on_map2(pos)) {
        return map2.color(world_to_map2(pos));
    } else {
        return 0.f;
    }
}

NORI_NAMESPACE_END