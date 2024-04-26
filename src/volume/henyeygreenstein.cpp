#include <core/common.h>
#include <volume/phase.h>
#include <stats/sampler.h>
#include <stats/warp.h>
#include <core/frame.h>

NORI_NAMESPACE_BEGIN

void HenyeyGreensteinPhase::sample(nori::Sampler *sampler, const nori::Vector3f &in, nori::Vector3f &out, float &pdf) const {
    Point2f sample = sampler->next2D();
    out = Warp::squareToHenyeyGreenstein(sample, g);
    out = Frame(in).toWorld(out);
    pdf = Warp::squareToHenyeyGreensteinPdf(in.dot(out), g);
}

Color3f nori::HenyeyGreensteinPhase::eval(const nori::Vector3f &in, const nori::Vector3f &out) const {
    return Warp::squareToHenyeyGreensteinPdf(in.dot(out), g);
}

std::string HenyeyGreensteinPhase::toString() const {
    return "hg";
}

float HenyeyGreensteinPhase::pdf(const Vector3f &in, const Vector3f &out) const {
    return Warp::squareToHenyeyGreensteinPdf(in.dot(out), g);
}

NORI_NAMESPACE_END