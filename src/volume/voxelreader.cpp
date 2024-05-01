#include <volume/coefficient.h>
#include <volume/vdbutil.h>

#include <utility>

NORI_NAMESPACE_BEGIN

static void compute_bounds(float x0, float x1, float y0, float y1, float z0, float z1, Point3f& min, Point3f& max) {
    min = Point3f(std::min(x0, x1), std::min(y0, y1), std::min(z0, z1));
    max = Point3f(std::max(x0, x1), std::max(y0, y1), std::max(z0, z1));
}

VoxelReader::VoxelReader(const std::string &path, Transform trafo, float d_factor)
    : transform(std::move(trafo)),
      voxel_data(from_file(path, trafo)),
      sampler(VSampler(*voxel_data)),
      d_factor(d_factor) {

    auto min = voxel_data->metaValue<openvdb::Vec3i>("file_bbox_min");
    auto max = voxel_data->metaValue<openvdb::Vec3i>("file_bbox_max");

    Vector3i min_i(min.x(), min.y(), min.z());
    Vector3i max_i(min.x(), min.y(), min.z());

    auto min_f = voxel_data->indexToWorld(openvdb::Coord(min));
    auto max_f = voxel_data->indexToWorld(openvdb::Coord(max));

    Point3f min_w;
    Point3f max_w;
    compute_bounds(min_f.x(), max_f.x(),
                   min_f.y(), max_f.y(),
                   min_f.z(),max_f.z(),
                   min_w, max_w);

    bounds_i = BoundingBox3i(min_i, max_i);
    bounds_w = BoundingBox3f(min_w, max_w);

    //std::cout << "Before: " << vstr(min_w) << "->" << vstr(max_w) << "\n";
    //std::cout << "After: " << vstr(bounds_w.min) << "->" << vstr(bounds_w.max) << "\n";

    auto minmax = openvdb::tools::minMax(voxel_data->tree());
    majorant = d_factor * minmax.max();

    inv_transform = transform.inverse();
}

float VoxelReader::eval(const nori::Point3f &p, const nori::Vector3f &v) const {
    // TODO: use cache version to optimize this
    Vector3f ip = p;
    return d_factor * sampler.wsSample(openvdb::Vec3R(ip.x(), ip.y(), ip.z()));
}

BinaryVoxelReader::BinaryVoxelReader(float value, const VoxelReader* child)
    : value(value), child(child) {
}

float BinaryVoxelReader::eval(const nori::Point3f &p, const nori::Vector3f &v) const {
    return child->eval(p, v) > Epsilon
        ? value
        : 0;
}

ScatteringVoxelReader::ScatteringVoxelReader(float max, const nori::VoxelReader *child) : max(max), child(child) {}


float ScatteringVoxelReader::eval(const nori::Point3f &p, const nori::Vector3f &v) const {
    float cv = child->eval(p, v);
    if(cv < Epsilon) return 0.f;
    if(cv < max / 3.f) return max;
    return max / 2;
}
NORI_NAMESPACE_END