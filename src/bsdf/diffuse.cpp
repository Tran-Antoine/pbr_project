/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob

    Nori is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Nori is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <bsdf/bsdf.h>
#include <core/frame.h>
#include <stats/warp.h>
#include <bsdf/diffusemap.h>
#include <bsdf/uniformmap.h>
#include <bsdf/texturemap.h>
#include <bsdf/graymap.h>
#include <bsdf/mountainmap.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Diffuse / Lambertian BRDF model
 */
class Diffuse : public BSDF {
public:
    Diffuse(const PropertyList &propList) {

        Color3f albedo = propList.getColor("albedo", -1.f);

        if(albedo.isValid()) {
            m_albedo = new UniformDiffuseMap(albedo);
        }
    }

    void addChild(nori::NoriObject *obj) override {
        switch (obj->getClassType()) {
            case NoriObject::EDiffuseMap:
                if(m_albedo) {
                    throw NoriException("Both albedo and diffuse maps were defined");
                }
                m_albedo = static_cast<DiffuseMap *>(obj);
                break;
            default:
                throw NoriException("Mesh::addChild(<%s>) is not supported!",
                                    classTypeName(obj->getClassType()));
        }
    }

    ~Diffuse() {
        delete m_albedo;
    }

    /// Evaluate the BRDF model
    Color3f eval(const BSDFQueryRecord &bRec) const {
        /* This is a smooth BRDF -- return zero if the measure
           is wrong, or when queried for illumination on the backside */
        if (bRec.measure != ESolidAngle
            || Frame::cosTheta(bRec.wi) <= 0
            || Frame::cosTheta(bRec.wo) <= 0)
            return Color3f(0.0f);

        // Texture coordinates are y-inverted
        Color3f albedo = m_albedo->T(bRec.uv.x(), 1-bRec.uv.y());
        /* The BRDF is simply the albedo / pi */
        return albedo * INV_PI;
    }

    /// Compute the density of \ref sample() wrt. solid angles
    float pdf(const BSDFQueryRecord &bRec) const {
        /* This is a smooth BRDF -- return zero if the measure
           is wrong, or when queried for illumination on the backside */
        if (bRec.measure != ESolidAngle
            || Frame::cosTheta(bRec.wi) <= 0
            || Frame::cosTheta(bRec.wo) <= 0)
            return 0.0f;


        /* Importance sampling density wrt. solid angles:
           cos(theta) / pi.

           Note that the directions in 'bRec' are in local coordinates,
           so Frame::cosTheta() actually just returns the 'z' component.
        */
        return INV_PI * Frame::cosTheta(bRec.wo);
    }

    /// Draw a a sample from the BRDF model
    Color3f sample(BSDFQueryRecord &bRec, const Point2f &sample) const {
        if (Frame::cosTheta(bRec.wi) <= 0)
            return Color3f(0.0f);

        bRec.measure = ESolidAngle;

        /* Warp a uniformly distributed sample on [0,1]^2
           to a direction on a cosine-weighted hemisphere */
        bRec.wo = Warp::squareToCosineHemisphere(sample);

        /* Relative index of refraction: no change */
        bRec.eta = 1.0f;

        /* eval() / pdf() * cos(theta) = albedo. There
           is no need to call these functions. */
        Color3f albedo = m_albedo->T(bRec.uv);
        return albedo;
    }

    bool isDiffuse() const {
        return true;
    }

    /// Return a human-readable summary
    std::string toString() const {
        return tfm::format(
            "Diffuse[\n"
            "  albedo = %s\n"
            "]", "todo"); // TODO Complete this
    }

    EClassType getClassType() const { return EBSDF; }
private:
    DiffuseMap* m_albedo = nullptr;
};

NORI_REGISTER_CLASS(Diffuse, "diffuse");
NORI_NAMESPACE_END
