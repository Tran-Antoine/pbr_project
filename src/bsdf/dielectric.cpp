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

NORI_NAMESPACE_BEGIN

/// Ideal dielectric BSDF
class Dielectric : public BSDF {
public:
    Dielectric(const PropertyList &propList) {
        /* Interior IOR (default: BK7 borosilicate optical glass) */
        m_intIOR = propList.getFloat("intIOR", 1.5046f);

        /* Exterior IOR (default: air) */
        m_extIOR = propList.getFloat("extIOR", 1.000277f);
    }

    Color3f eval(const BSDFQueryRecord &) const {
        /* Discrete BRDFs always evaluate to zero in Nori */
        return Color3f(0.0f);
    }

    float pdf(const BSDFQueryRecord &) const {
        /* Discrete BRDFs always evaluate to zero in Nori */
        return 0.0f;
    }

    Color3f sample(BSDFQueryRecord &bRec, const Point2f &sample) const {

        float inv_factor = 1;
        Vector3f n(0.0f, 0.0f, 1.0f);

        Vector3f wi = bRec.wi.normalized();

        float cos_theta_i, cos_theta_t;
        float eta_from, eta_to;
        Vector3f wr, wt;

        wr = Vector3f(-wi.x(), -wi.y(), wi.z());

        if(wi.z() < 0) {
            eta_from = m_intIOR;
            eta_to = m_extIOR;
            n = -n;
            inv_factor = -1;
        } else {
            eta_from = m_extIOR;
            eta_to = m_intIOR;
        }

        cos_theta_i = inv_factor * wi.z();
        
        float sqrt_term = 1 - pow(eta_from/eta_to, 2) * (1 - pow(cos_theta_i, 2));

        if(sqrt_term < 0) {
            bRec.wo = wr;
            return Color3f(1.0f);
        }

        // here it's simpler to use n instead of adding z commponents to vectors
        wt = - eta_from / eta_to * (wi - (cos_theta_i*n)) - n*sqrt(sqrt_term);
        cos_theta_t = -inv_factor * wt.z();

        float f_r, f_t;

        // fresnel's equations 
        float rho_paral = (eta_to * cos_theta_i - eta_from * cos_theta_t) / (eta_to * cos_theta_i + eta_from * cos_theta_t);
        float rho_perp  = (eta_from * cos_theta_i - eta_to * cos_theta_t) / (eta_from * cos_theta_i + eta_to * cos_theta_t);
        f_r = 0.5 * (rho_paral*rho_paral + rho_perp*rho_perp);
        f_t = 1 - f_r;

        if(sample.x() < f_r) {
            bRec.wo = wr;
            return Color3f(1.0f);
        } else {
            bRec.wo = wt;
            return Color3f(1.0f);
        }
    }

    std::string toString() const override {
        return tfm::format(
            "Dielectric[\n"
            "  intIOR = %f,\n"
            "  extIOR = %f\n"
            "]",
            m_intIOR, m_extIOR);
    }
private:
    float m_intIOR, m_extIOR;
};

NORI_REGISTER_CLASS(Dielectric, "dielectric");
NORI_NAMESPACE_END
