#pragma once

#include <core/common.h>
#include <core/object.h>
#include <core/vector.h>
#include <volume/phase.h>
#include <openvdb/openvdb.h>
#include <openvdb/tools/Interpolation.h>
#include <core/bbox.h>
#include <core/transform.h>
#include <core/color.h>

#include <utility>

NORI_NAMESPACE_BEGIN

class MediumCoefficient {

public:
    virtual Color3f eval(const Point3f& p, const Vector3f& v) const { return 0.f; }
    virtual float maj() const { return 0.f; };
};

class ConstantCoefficient : public MediumCoefficient {
public:
    explicit ConstantCoefficient(Color3f coeff) : coeff(std::move(coeff)) {}

    Color3f eval(const Point3f& p, const Vector3f& v) const override { return coeff; }
    float maj() const override { return coeff.maxCoeff(); }
private:
    Color3f coeff;
};

using VGrid = openvdb::FloatGrid;
using VSampler = openvdb::tools::GridSampler<VGrid, openvdb::tools::BoxSampler>;

class VoxelReader : public MediumCoefficient {
public:
    VoxelReader(const std::string &path, Transform transform, float d_factor, const Color3f& albedo=Color3f(1));
    Color3f eval(const Point3f& p, const Vector3f& v) const override;

    BoundingBox3i index_bounds() const { return bounds_i; }
    BoundingBox3f world_bounds() const { return bounds_w; }
    float maj() const override { return majorant; }

protected:
    VGrid::Ptr voxel_data;
    VSampler sampler;
    Transform transform;
    Transform inv_transform;
    float d_factor;
    float majorant;
    BoundingBox3i bounds_i;
    BoundingBox3f bounds_w;
    Color3f albedo;
};

class BinaryVoxelReader : public MediumCoefficient {
public:
    BinaryVoxelReader(Color3f value, const VoxelReader* child);

    BoundingBox3i index_bounds() const { return child->index_bounds(); }
    BoundingBox3f world_bounds() const { return child->world_bounds(); }
    float maj() const override { return value.maxCoeff(); }
    Color3f eval(const Point3f& p, const Vector3f& v) const override;

private:
    Color3f value;
    const VoxelReader* child = nullptr;
};

class ScatteringVoxelReader : public MediumCoefficient {
public:
    ScatteringVoxelReader(float max, const VoxelReader* child);

    BoundingBox3i index_bounds() const { return child->index_bounds(); }
    BoundingBox3f world_bounds() const { return child->world_bounds(); }
    float maj() const override { return max; }
    Color3f eval(const Point3f& p, const Vector3f& v) const override;

private:
    float max;
    const VoxelReader* child = nullptr;
};

NORI_NAMESPACE_END
