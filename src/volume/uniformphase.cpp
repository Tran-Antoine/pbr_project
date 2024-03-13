#include <core/common.h>
#include <volume/phase.h>
#include <stats/sampler.h>
#include <stats/warp.h>

NORI_NAMESPACE_BEGIN

void UniformPhase::eval(nori::Sampler *sampler, const nori::Vector3f &in, nori::Vector3f &out, float &pdf) const {
    Point2f sample = sampler->next2D();
    out = Warp::squareToUniformSphere(sample);
    pdf = Warp::squareToUniformSpherePdf(out);
}

std::string UniformPhase::toString() const {
    return "uniform";
}

NORI_NAMESPACE_END