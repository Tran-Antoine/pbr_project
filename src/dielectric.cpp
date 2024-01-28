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

#include <nori/bsdf.h>
#include <nori/frame.h>

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

        // TODO : use the record's eta?
        float eta1 = m_extIOR;
        float eta2 = m_intIOR;


        Vector3f wi = bRec.wi;

        Vector3f n(0.0f, 0.0f, 1.0f);

        // if wi is in the same direction as n, it means the light is exiting the surface, not entering it (so no refraction)
        if(Frame::cosTheta(wi) > 0) {
            bRec.wo = -wi;
            return Color3f(1.0f);
        }

        // snell's law
        Vector3f w_refracted = - eta1 / eta2 * (wi - (wi.dot(n)*n) - n * sqrt(1 - pow(eta1/eta2, 2)*(1 - pow(wi.dot(n), 2))));

        float cos_theta_1 = wi.z(); //wi.dot(n);


        // why is this value negative ??
        float cos_theta_2 = -w_refracted.z(); //-w_refracted.dot(n);


        std::cout << cos_theta_1 << "\n";
        
        // fresnel's equations 
        float rho_paral = (eta2 * cos_theta_1 - eta1 * cos_theta_2) / (eta2 * cos_theta_1 + eta1 * cos_theta_2);
        float rho_perp  = (eta1 * cos_theta_1 - eta2 * cos_theta_2) / (eta1 * cos_theta_1 + eta2 * cos_theta_2);
        float f_r = 0.5 * (rho_paral*rho_paral + rho_perp*rho_perp);
        float f_t = 1 - f_r;

        if(sample.x() < f_r) {
            bRec.wo = Vector3f(
                -bRec.wi.x(),
                -bRec.wi.y(),
                bRec.wi.z()
            );
            return Color3f(1 / f_r * -cos_theta_2);
        } else {
            bRec.wo = w_refracted;
            return Color3f(1 / f_t * -cos_theta_2);
        }
    }

    std::string toString() const {
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
