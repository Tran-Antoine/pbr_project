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

#pragma once

#include <core/common.h>
#include <stats/sampler.h>
#include <collection/mipmap.h>

NORI_NAMESPACE_BEGIN

/// A collection of useful warping functions for importance sampling
class Warp {
public:
    /// Dummy warping function: takes uniformly distributed points in a square and just returns them
    static Point2f squareToUniformSquare(const Point2f &sample);


    /// Probability density of \ref squareToUniformSquare()
    static float squareToUniformSquarePdf(const Point2f &p);

    /// Sample a 2D tent distribution
    static Point2f squareToTent(const Point2f &sample);

    /// Probability density of \ref squareToTent()
    static float squareToTentPdf(const Point2f &p);

    /// @brief Sample a point uniformly on a mesh.
    /// @param sample0 a 1D uniformly distributed point between 0 and 1, used for triangle selection
    /// @param sample1 a 2D uniformly distributed point with coordinates between 0 and 1, used for point selection
    /// @param mesh the mesh of reference
    /// @param p upon success, will contain the sampled 3D position
    /// @param n upon success, will contain the surface normal of position p
    /// @param pdf upon success, will contain the density function of a point on the mesh chosen u.a.r, as a constant
    static void squareToMeshPoint(const Point2f &sample0, const Mesh &mesh, Point3f &p, Vector3f &n, float &pdf);


    static float squareToMeshPointPdf(const Vector3f &v, const Mesh &mesh);

    /// Uniformly sample a vector on a 2D disk with radius 1, centered around the origin
    static Point2f squareToUniformDisk(const Point2f &sample);

    /// Probability density of \ref squareToUniformDisk()
    static float squareToUniformDiskPdf(const Point2f &p);

    /// Uniformly sample a vector on the unit sphere with respect to solid angles
    static Vector3f squareToUniformSphere(const Point2f &sample);

    /// Probability density of \ref squareToUniformSphere()
    static float squareToUniformSpherePdf(const Vector3f &v);

    /// Uniformly sample a vector on the unit hemisphere around the pole (0,0,1) with respect to solid angles
    static Vector3f squareToUniformHemisphere(const Point2f &sample);

    /// Probability density of \ref squareToUniformHemisphere()
    static float squareToUniformHemispherePdf(const Vector3f &v);

    /// Uniformly sample a vector on the unit hemisphere around the pole (0,0,1) with respect to projected solid angles
    static Vector3f squareToCosineHemisphere(const Point2f &sample);

    /// Probability density of \ref squareToCosineHemisphere()
    static float squareToCosineHemispherePdf(const Vector3f &v);

    /// Warp a uniformly distributed square sample to a Beckmann distribution * cosine for the given 'alpha' parameter
    static Vector3f squareToBeckmann(const Point2f &sample, float alpha);

    /// Probability density of \ref squareToBeckmann()
    static float squareToBeckmannPdf(const Vector3f &m, float alpha);

    /// Warp a uniformly distributed square sample to a hierarchical distribution from a gray-scale map
    static Point2f squareToGrayMap(const Point2f &sample, const MipMap& map);

    /// Probability density of \ref squareToGrayMap()
    static float squareToGrayMapPdf(const Point2f &p, const MipMap& map);

    static float lineToHomogeneousPath(float s, float omega_t);

    static float lineToHomogeneousPathPdf(float t, float omega_t);

    static float lineToLogistic(float s, float std);

    static float lineToLogisticPdf(float t, float std);

    static float sampleHeterogeneousDistance(Sampler* sampler, const Point3f& x, const Vector3f& d, const Medium &medium, float& pdf);

    static float sampleToHeterogeneousPathPdf(const Point3f& a, const Point3f& b, const Medium &medium, Sampler* sampler);

    static float ratio_tracking(const Point3f &a, const Point3f &b, const Medium &medium, Sampler *sampler);

    static Vector3f squareToHenyeyGreenstein(const Point2f& sample, float g);

    static Vector3f squareToHenyeyGreensteinPdf(float cosTheta, float g);
};

NORI_NAMESPACE_END
