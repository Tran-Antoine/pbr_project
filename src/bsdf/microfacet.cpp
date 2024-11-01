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
#include <bsdf/texturemap.h>
#include <bsdf/uniformmap.h>

NORI_NAMESPACE_BEGIN

class Microfacet : public BSDF {
public:
    Microfacet(const PropertyList &propList) {
        /* RMS surface roughness */
        m_alpha = propList.getFloat("alpha", 0.1f);

        /* Interior IOR (default: BK7 borosilicate optical glass) */
        m_intIOR = propList.getFloat("intIOR", 1.5046f);

        /* Exterior IOR (default: air) */
        m_extIOR = propList.getFloat("extIOR", 1.000277f);

        Color3f albedo = propList.getColor("kd", Color3f(-1.f));

        if(albedo.isValid()) {
            m_albedo = new UniformDiffuseMap(albedo);
        }

        /* To ensure energy conservation, we must scale the 
           specular component by 1-kd. 

           While that is not a particularly realistic model of what 
           happens in reality, this will greatly simplify the 
           implementation. Please see the course staff if you're 
           interested in implementing a more realistic version 
           of this BRDF. */
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

    Color3f G(Vector3f wi, Vector3f wo, Vector3f wh) const {
        auto G1 = [this](Vector3f wv, Vector3f wh) -> float {

            float b = 1 / (m_alpha * Frame::tanTheta(wv));

            float coeff = b < 1.6f ? (3.535f*b + 2.181f*b*b) / (1.0f + 2.276f*b + 2.577f*b*b) : 1.0f;

            float c = wv.dot(wh) / wv.z();

            float xplus = c > 0.0f ? 1.0f : 0.0f;

            return xplus * coeff;
        };

        return Color3f(G1(wi, wh) * G1(wo, wh));
    }

    float F(float cos_theta_i, float eta_to, float eta_from) const {
        return fresnel(cos_theta_i, eta_to, eta_from);
    }

    float D(Vector3f wh) const {
        return Warp::squareToBeckmannPdf(wh, m_alpha);
    }
    /// Evaluate the BRDF for the given pair of directions
    Color3f eval(const BSDFQueryRecord &bRec) const {
    	
        if (bRec.measure != ESolidAngle
            || Frame::cosTheta(bRec.wi) <= 0
            || Frame::cosTheta(bRec.wo) <= 0)
            return Color3f(0.0f);

        Vector3f wi = bRec.wi;
        Vector3f wo = bRec.wo;
        Vector3f wh = (wi + wo).normalized();

        float cos_theta_i = Frame::cosTheta(wi), 
              cos_theta_o = Frame::cosTheta(wo), 
              cos_theta_h = Frame::cosTheta(wh);

        Color3f kd = m_albedo->T(bRec.uv.x(), 1 - bRec.uv.y());
        float ks = 1 - std::max({kd.x(), kd.y(), kd.z()});

        Color3f out = kd / M_PI + ks * (D(wh) * F(wh.dot(wi), m_extIOR, m_intIOR) * G(wi, wo, wh)) / (4*cos_theta_i*cos_theta_o*cos_theta_h);

        return out;
    }

    /// Evaluate the sampling density of \ref sample() wrt. solid angles
    float pdf(const BSDFQueryRecord &bRec) const {
        if (bRec.measure != ESolidAngle
            || Frame::cosTheta(bRec.wi) <= 0
            || Frame::cosTheta(bRec.wo) <= 0)
            return 0.0f;

        Color3f m_kd = m_albedo->T(bRec.uv.x(), 1 - bRec.uv.y());
    	float ks = 1 - std::max({m_kd.x(), m_kd.y(), m_kd.z()});
        Vector3f wh = (bRec.wi + bRec.wo).normalized();
        
        return ks * Warp::squareToBeckmannPdf(wh, m_alpha) / (4*wh.dot(bRec.wo)) + (1 - ks) * Warp::squareToCosineHemispherePdf(bRec.wo);
    }

    /// Sample the BRDF
    Color3f sample(BSDFQueryRecord &bRec, const Point2f &_sample) const {
    	
        bRec.measure = EMeasure::ESolidAngle;

        float eps1 = _sample.x();
        float eps2 = _sample.y();

        Color3f m_kd = m_albedo->T(bRec.uv.x(), 1 - bRec.uv.y());
        float ks = 1 - std::max({m_kd.x(), m_kd.y(), m_kd.z()});

        if(eps1 < ks) { // handle specular
            eps1 = eps1 / ks;

            Vector3f wi = bRec.wi;

            Vector3f wh = Warp::squareToBeckmann(Point2f(eps1, eps2), m_alpha);
      
            Vector3f wo = 2 * wh.dot(wi) * wh - wi;

            bRec.wo = wo;

        } else { // handle diffuse
            eps1 = (eps1 - ks) / (1 - ks);
            bRec.wo = Warp::squareToCosineHemisphere(Point2f(eps1, eps2));
        } 
        
        Color3f output = eval(bRec);

        if(output.isZero()) {
            return output;
        }

        // max(0, x) to avoid values such as -10E-7
        return output * std::max(0.0f, Frame::cosTheta(bRec.wo) / pdf(bRec));

        // Note: Once you have implemented the part that computes the scattered
        // direction, the last part of this function should simply return the
        // BRDF value divided by the solid angle density and multiplied by the
        // cosine factor from the reflection equation, i.e.
        // return eval(bRec) * Frame::cosTheta(bRec.wo) / pdf(bRec);
    }

    bool isDiffuse() const {
        /* While microfacet BRDFs are not perfectly diffuse, they can be
           handled by sampling techniques for diffuse/non-specular materials,
           hence we return true here */
        return true;
    }

    std::string toString() const {
        return tfm::format(
            "Microfacet[\n"
            "  alpha = %f,\n"
            "  intIOR = %f,\n"
            "  extIOR = %f,\n"
            "  kd = %s,\n"
            "  ks = %f\n"
            "]",
            m_alpha,
            m_intIOR,
            m_extIOR,
            "",
            m_ks
        );
    }
private:
    float m_alpha;
    float m_intIOR, m_extIOR;
    float m_ks;
    DiffuseMap* m_albedo = nullptr;
};

NORI_REGISTER_CLASS(Microfacet, "microfacet");
NORI_NAMESPACE_END
