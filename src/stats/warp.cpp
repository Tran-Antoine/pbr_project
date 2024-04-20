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

#include <core/common.h>
#include <stats/warp.h>
#include <core/vector.h>
#include <core/frame.h>
#include <core/mesh.h>
#include <algorithm>
#include <iomanip>
#include <stats/dpdf.h>
#include <Eigen/Geometry>

NORI_NAMESPACE_BEGIN

Point2f Warp::squareToUniformSquare(const Point2f &sample) {
    return sample;
}

float Warp::squareToUniformSquarePdf(const Point2f &sample) {
    return ((sample.array() >= 0).all() && (sample.array() <= 1).all()) ? 1.0f : 0.0f;
}

Point2f Warp::squareToTent(const Point2f &sample) {

    auto inv_pcf_1d = [](float t) -> float {
        if(t < 0 || t > 1) throw NoriException("Value outside [0, 1]");
        if(t <= 0.5) return -1 + sqrt(2*t);
        else         return  1 - sqrt(2*(1-t));
    };

    // making x non-uniform to make up for the distortion of the mapping
    float new_x = inv_pcf_1d(sample.x());

    // normal mapping
    float new_y = sample.y() * (1 - abs(new_x));

    return Point2f(new_x, new_y);
} 

float Warp::squareToTentPdf(const Point2f &p) {

    if(p.x() < -1 || p.x() > 1) return 0;

    if(p.y() > 1 - abs(p.x()) || p.y() < 0) return 0; 

    return 1;
}

void Warp::squareToMeshPoint(const Point2f &sample0, const Mesh &mesh, Point3f &p, Vector3f &n, float &pdf) {

    DiscretePDF* mesh_pdf = mesh.getTrianglePDF();

    float eps1 = sample0.x();
    float eps2 = sample0.y();
    
    // crucial to use "reusue", otherwise triangle selection and barycentral coordinates are NOT independent
    uint32_t chosen_index = mesh_pdf->sampleReuse(eps1);

    float bary0 = 1 - sqrt(1 - eps1);
    float bary1  = eps2 * sqrt(1 - eps1);
    float bary2 = 1 - bary0 - bary1;
    
    auto m_F = mesh.getIndices();
    auto m_V = mesh.getVertexPositions();

    uint32_t i0 = m_F(0, chosen_index), i1 = m_F(1, chosen_index), i2 = m_F(2, chosen_index);
    const Point3f p0 = m_V.col(i0), p1 = m_V.col(i1), p2 = m_V.col(i2);

    Point3f sampled_point = bary0 * p0 + bary1 * p1 + bary2 * p2;

    // TODO: Move this section into the mesh source file
    auto m_N = mesh.getVertexNormals();

    Vector3f normal;
    
    if(m_N.size() > 0) {
        normal = (bary0 * m_N.col(i0) +
                    bary1 * m_N.col(i1) +
                    bary2 * m_N.col(i2)).normalized();
    } else {
        normal = (p1-p0).cross(p2-p0).normalized();
    }
    //

    p = sampled_point;
    n = normal;
    pdf = 1 / mesh.getTotalArea();
}

float Warp::squareToMeshPointPdf(const Vector3f &v, const Mesh &mesh) {
    return 1 / mesh.getTotalArea();
}

Point2f Warp::squareToUniformDisk(const Point2f &sample) {

    float new_x = sqrt(sample.x());
    float new_y = 2*M_PI*sample.y();

    float mapped_x = new_x * cos(new_y);
    float mapped_y = new_x * sin(new_y);

    return Point2f(mapped_x, mapped_y);
}

float Warp::squareToUniformDiskPdf(const Point2f &p) {
    
    float x = p.x();
    float y = p.y();

    if(x*x + y*y < 1) return 1 / M_PI;
    return 0;
}

Vector3f Warp::squareToUniformSphere(const Point2f &sample) {
    // formula from the lecture notes
    float eps_1 = sample.x();
    float eps_2 = sample.y();
    
    float z = 2*eps_1 - 1;
    float r = sqrt(1-z*z);
    float phi = 2*M_PI*eps_2;
    float x = r*cos(phi);
    float y = r*sin(phi);

    return Vector3f(x,y,z);
}

float Warp::squareToUniformSpherePdf(const Vector3f &v) {
    
    float x = v.x();
    float y = v.y();
    float z = v.z();

    // inverting area of r=1 sphere of value 4pi
    if(abs(x*x + y*y + z*z - 1) < 1e-6) return 1 / (4*M_PI);
    return 0;
}

Vector3f Warp::squareToUniformHemisphere(const Point2f &sample) {
    float eps_1 = sample.x();
    float eps_2 = sample.y();
    
    float z = 2*eps_1 - 1;
    float r = sqrt(1-z*z);
    float phi = 2*M_PI*eps_2;
    float x = r*cos(phi);
    float y = r*sin(phi);

    return Vector3f(x,y,abs(z));
}

float Warp::squareToUniformHemispherePdf(const Vector3f &v) {

    float x = v.x();
    float y = v.y();
    float z = v.z();

    if(z < 0) return 0;

    // inverting area of r=1 half sphere of value 2pi
    if(abs(x*x + y*y + z*z - 1) < 1e-6) return 1 / (2*M_PI);
    return 0;
}

Vector3f Warp::squareToCosineHemisphere(const Point2f &sample) {
    Point2f disk_mapping = squareToUniformDisk(sample);

    float x = disk_mapping.x();
    float y = disk_mapping.y();

    float z = sqrt(1 - x*x - y*y);
    
    return Vector3f(x,y,z);
}

float Warp::squareToCosineHemispherePdf(const Vector3f &v) {
    
    float x = v.x();
    float y = v.y();
    float z = v.z();

    if(z < 0) return 0;
    if(abs(x*x + y*y + z*z - 1) > Epsilon) return 0;

    float cos_theta = z;
    return cos_theta / M_PI;
}

Vector3f Warp::squareToBeckmann(const Point2f &sample, float alpha) {
    
    float eps1 = sample.x();
    float eps2 = sample.y();

    float phi = 2*M_PI*eps1;
    float tan2_theta = -log(1-eps2)*alpha*alpha; // no need to compute theta directly (cf pbr book)

    float sin_phi = sin(phi);
    float cos_phi = cos(phi);
    float cos_theta = 1 / sqrt(1 + tan2_theta);
    float sin_theta = sqrt(std::max(0.0f, 1 - cos_theta * cos_theta));

    Vector3f wh = Vector3f(sin_theta*cos_phi, sin_theta*sin_phi, cos_theta);

    return wh;
}

float Warp::squareToBeckmannPdf(const Vector3f &m, float alpha) {
    
    float cos_theta = Frame::cosTheta(m); 
    float tan_theta = Frame::tanTheta(m);

    if(cos_theta < 0) return 0.f;

    float pdf = 1 / (2 * M_PI) * (2 * exp(-pow(tan_theta, 2) / (alpha * alpha)) / (alpha * alpha * pow(cos_theta, 3)));

    return std::max(0.0f, pdf);
}

static bool pick_reuse(float& sample, float p, float q) {
    if(sample < p) {
        sample /= p;
        return true;
    } else {
        sample = (sample - p) / q;
        return false;
    }
}

Point2f Warp::squareToGrayMap(const Point2f &sample, const MipMap &map) {

    Point2f current_sample = Point2f(sample);
    Point2i current_corner;

    for(int depth = 1; depth <= map.depth(); depth++) {
        
        current_corner = map.next_corner(depth, current_corner);

        float up, down;
        map.v_distribution(current_corner, up, down);
        bool down_picked = pick_reuse(current_sample.y(), down, up);

        float left, right;
        map.h_distribution(current_corner, down_picked, left, right);
        bool left_picked = pick_reuse(current_sample.x(), left, right);

        MipMap::Quadrant quadrant = MipMap::quadrant(!down_picked, left_picked);
        MipMap::move(current_corner, quadrant);
    }

    float final_x = (float) current_corner.x() + current_sample.x();
    float final_y = (float) current_corner.y() + current_sample.y();

    Point2f p = Point2f(final_x / (map.max_param() + 1.f), final_y / (map.max_param() + 1.f));

    return p;
}

float Warp::squareToGrayMapPdf(const Point2f &p, const MipMap &map) {

    int x = (int) (p.x() * map.max_param());
    int y = (int) (p.y() * map.max_param());

    float value = map.grayscale(x, y);

    if(map.is_normalized()) {
        return value;
    } else {
        return value / map.get_luminance();
    }
}

float Warp::lineToHomogeneousPath(float s, float omega_t) {
    return - log(1 - s) / omega_t;
}

float Warp::lineToHomogeneousPathPdf(float t, float omega_t) {
    return omega_t * exp(-omega_t * t);
}

float Warp::lineToLogistic(float s, float std) {
    return -std * std::log(1 / s - 1);
}

float Warp::lineToLogisticPdf(float t, float std) {
    throw NoriException("Not implemented yet");
}

static float majorant(const Medium& medium) {
    throw NoriException("Not implemented yet");
}

float Warp::sampleHeterogeneousDistance(Sampler* sampler, const Point3f& x, const Vector3f& d, const Medium &medium, float& pdf) {

    float eta1 = sampler->next1D();
    float maj = majorant(medium);

    float t = Epsilon;
    float p_real_interaction = 0.f;

    float _pdf = 1.f;

    while(p_real_interaction < eta1 && medium.bounds().contains(x + t*d)) {

        _pdf *= (1 - p_real_interaction);

        float eta2 = sampler->next1D();
        float distance_travelled = lineToHomogeneousPath(eta2, maj);

        t += distance_travelled;

        float omega_t = medium.attenuation(x + t * d, d);
        p_real_interaction = omega_t / maj;

        // remapping
        eta1 = (eta1 - p_real_interaction) / (1 - p_real_interaction);
    }

    pdf = _pdf;
    return t;
}

float Warp::ratio_tracking(const Point3f& a, const Point3f& b, const Medium& medium, Sampler* sampler) {
    Vector3f d = (b-a).normalized();
    float total_distance = (b-a).norm();
    float maj = majorant(medium);

    float weight = 1.f;
    float t = 0.f;

    while(t < total_distance) {
        t += -log(1 - sampler->next1D()) / maj;
        float null_amount = maj - medium.attenuation(a + d*t, d);
        weight *= null_amount / maj;
    }

    return weight;
}

float Warp::sampleToHeterogeneousPathPdf(const Point3f& a, const Point3f& b, const Medium &medium, Sampler* sampler) {
    throw NoriException("PDF cannot be a posteriori evaluated");
}

Vector3f Warp::squareToHenyeyGreenstein(const Point2f& sample, float g) {
    float temp = (1 - g*g) / (1 - g + 2*g*sample.x());
    float cosTheta = 1 / (2*g) * (1 + g*g - (temp*temp));
    float sinTheta = sqrt(1 - cosTheta*cosTheta);
    float phi = 2*M_PI*sample.y();

    return Vector3f(
            sinTheta * cos(phi),
            sinTheta * sin(phi),
            cosTheta
    );
}

Vector3f Warp::squareToHenyeyGreensteinPdf(float cosTheta, float g) {
    return 1.f / (4*M_PI) * (1 - g*g) / pow(1 + g*g + 2*g*cosTheta, 1.5);
}

NORI_NAMESPACE_END
