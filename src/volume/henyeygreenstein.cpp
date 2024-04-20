#include <core/common.h>
#include <volume/phase.h>
#include <stats/sampler.h>
#include <stats/warp.h>

NORI_NAMESPACE_BEGIN

void HenyeyGreensteinPhase::sample(nori::Sampler *sampler, const nori::Vector3f &in, nori::Vector3f &out, float &pdf) const {
    Point2f sample = sampler->next2D();
    out = Warp::squareToUniformSphere(sample);
    pdf = Warp::squareToUniformSpherePdf(out);
}

Color3f nori::HenyeyGreensteinPhase::eval(const nori::Vector3f &in, const nori::Vector3f &out) const {
    return Color3f(1.0f / (4*M_PI));
}

std::string HenyeyGreensteinPhase::toString() const {
    return "uniform";
}

float HenyeyGreensteinPhase::pdf(const Vector3f &in, const Vector3f &out) const {
    return 1.0f / (4*M_PI);
}

NORI_NAMESPACE_END