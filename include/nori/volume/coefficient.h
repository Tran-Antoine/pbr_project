#pragma once

#include <core/common.h>
#include <core/object.h>
#include <core/vector.h>
#include <volume/phase.h>
#include <openvdb/openvdb.h>
#include <openvdb/tools/Interpolation.h>
#include <core/bbox.h>
#include <core/transform.h>

NORI_NAMESPACE_BEGIN

class MediumCoefficient {

public:
    virtual float eval(const Point3f& p, const Vector3f& v) const { return 0.f; }
    virtual float maj() const { return 0.f; };
};

class ConstantCoefficient : public MediumCoefficient {
public:
    explicit ConstantCoefficient(float coeff) : coeff(coeff) {}

    float eval(const Point3f& p, const Vector3f& v) const override { return coeff; }
    float maj() const override { return coeff; }
private:
    float coeff;
};

using VGrid = openvdb::FloatGrid;
using VSampler = openvdb::tools::GridSampler<VGrid, openvdb::tools::BoxSampler>;

class VoxelReader : public MediumCoefficient {
public:
    VoxelReader(const std::string &path, Transform  transform, float d_factor);
    float eval(const Point3f& p, const Vector3f& v) const override;

    BoundingBox3i index_bounds() const { return bounds_i; }
    BoundingBox3f world_bounds() const { return bounds_w; }
    float maj() const override { return majorant; }

protected:
    VGrid::Ptr voxel_data;
    VSampler sampler;
    Transform transform;
    float d_factor;
    float majorant;
    BoundingBox3i bounds_i;
    BoundingBox3f bounds_w;
};

NORI_NAMESPACE_END
