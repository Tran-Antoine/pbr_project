#pragma once

#include <core/common.h>
#include <core/bbox.h>
#include <vector>
#include <core/vector.h>

NORI_NAMESPACE_BEGIN

struct ProceduralMetadata {
    float radius;
    BoundingBox3f bounds;
    Vector3i resolution;
    std::string out_path;
};

void write_vdb(const std::vector<Point3f>& positions, const ProceduralMetadata& metadata);

NORI_NAMESPACE_END
