#pragma once

#include <core/common.h>
#include <core/bbox.h>
#include <vector>
#include <core/vector.h>

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


NORI_NAMESPACE_END