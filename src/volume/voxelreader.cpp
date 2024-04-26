#include <volume/coefficient.h>
#include <volume/vdbutil.h>

#include <utility>

NORI_NAMESPACE_BEGIN

VoxelReader::VoxelReader(const std::string &path, Transform trafo, float d_factor)
    : transform(std::move(trafo)), voxel_data(from_file(path)), sampler(VSampler(*voxel_data)), d_factor(d_factor) {

    auto min = voxel_data->metaValue<openvdb::Vec3i>("file_bbox_min");
    auto max = voxel_data->metaValue<openvdb::Vec3i>("file_bbox_max");

    Vector3i min_i(min.x(), min.y(), min.z());
    Vector3i max_i(min.x(), min.y(), min.z());

    auto min_f = voxel_data->indexToWorld(openvdb::Coord(min));
    auto max_f = voxel_data->indexToWorld(openvdb::Coord(max));

    Vector3f min_w(min_f.x(), min_f.y(), min_f.z());
    Vector3f max_w(max_f.x(), max_f.y(), max_f.z());

    bounds_i = BoundingBox3i(min_i, max_i);
    bounds_w = BoundingBox3f(min_w, max_w);

    auto minmax = openvdb::tools::minMax(voxel_data->tree());
    majorant = d_factor * minmax.max();
}

float VoxelReader::eval(const nori::Point3f &p, const nori::Vector3f &v) const {
    // TODO: use cache version to optimize this
    return d_factor * sampler.wsSample(openvdb::Vec3R(p.x(), p.y(), p.z()));
}

NORI_NAMESPACE_END