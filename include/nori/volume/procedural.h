#pragma once

#include <core/common.h>
#include <core/bbox.h>
#include <vector>
#include <core/vector.h>
#include <core/transform.h>

NORI_NAMESPACE_BEGIN

struct ProceduralMetadata {
    float radius;
    float radius_bg;
    BoundingBox3f bounds;
    Vector3i resolution;
    std::string out_path;
};

void write_vdb(const std::vector<Point3f>& positions, const ProceduralMetadata& metadata);

void write_vdb(const std::vector<Point3f>& main_positions, const std::vector<Point3f>& bg_positions, const ProceduralMetadata& metadata);

void write_spiral(const Vector3i& res, const Transform& trafo, const std::string& output_path);

void write_sky(const Vector3i& n_clouds, const Vector3i& voxel_res, const BoundingBox3f& bounds, const Point3f& hole, float hole_radius, const std::string& output_path);

void write_sky(const Vector3i& n_clouds, const Vector3i& voxel_res, const BoundingBox3f& bounds, const std::vector<Point3f>& holes, const std::vector<float>& hole_radii, const std::string& output_path);

NORI_NAMESPACE_END
