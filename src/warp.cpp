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

#include <nori/warp.h>
#include <nori/vector.h>
#include <nori/frame.h>
#include <nori/mesh.h>
#include <algorithm>
#include <iomanip>
#include <nori/dpdf.h>

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

static void cubeToMeshPoint(const float sample0, const Point2f &sample1, const Mesh &mesh, Point3f &p, Vector3f &n, float &pdf) {

    // TODO: dont init and fill every time
    DiscretePDF mesh_pdf(mesh.getTriangleCount());

    float total_area = 0;

    for(uint32_t i = 0; i < mesh.getTriangleCount(); i++) {
        float area = mesh.surfaceArea(i);
        mesh_pdf.append(area);
        total_area += area;
    }
    //
    
    uint32_t chosen_index = mesh_pdf.sample(sample0);

    float eps1 = sample1.x();
    float eps2 = sample1.y();

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
    pdf = 1 / total_area;
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
    if(abs(x*x + y*y + z*z - 1) > 1e-5) return 0;

    float cos_theta = z;
    return cos_theta / M_PI;
}

Vector3f Warp::squareToBeckmann(const Point2f &sample, float alpha) {
    throw NoriException("Warp::squareToBeckmann() is not yet implemented!");
}

float Warp::squareToBeckmannPdf(const Vector3f &m, float alpha) {
    throw NoriException("Warp::squareToBeckmannPdf() is not yet implemented!");
}

NORI_NAMESPACE_END
