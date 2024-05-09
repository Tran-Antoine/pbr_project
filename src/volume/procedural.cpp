#include <volume/procedural.h>
#include <openvdb/openvdb.h>
#include <openvdb/tools/Interpolation.h>
#include <pcg32.h>

NORI_NAMESPACE_BEGIN

static int count(const std::vector<Point3f>& positions, const openvdb::math::Vec3<double>& v, float radius) {
    int c = 0;
    for(auto p : positions) {
        float x0 = p.x(), y0 = p.y(), z0 = p.z();
        float x = v.x(), y = v.y(), z = v.z();
        if((x0 - x) * (x0 - x) + (y0 - y) * (y0 - y) + (z0 - z) * (z0 - z) < radius * radius) {
            c++;
        }
    }
    return c;
}

void write_vdb(const std::vector<Point3f>& positions, const ProceduralMetadata& metadata) {

    pcg32 random;

    Vector3i res = metadata.resolution;

    Vector3f min_pos = metadata.bounds.min;
    Vector3f max_pos = metadata.bounds.max;

    float scale_x = (max_pos.x() - min_pos.x()) / res.x();
    float scale_y = (max_pos.y() - min_pos.y()) / res.y();
    float scale_z = (max_pos.z() - min_pos.z()) / res.z();

    float translate_x = (max_pos.x() - min_pos.x()) * (0.5 + min_pos.x() / (max_pos.x() - min_pos.x()));
    float translate_y = (max_pos.y() - min_pos.y()) * (0.5 + min_pos.y() / (max_pos.y() - min_pos.y()));
    float translate_z = (max_pos.z() - min_pos.z()) * (0.5 + min_pos.z() / (max_pos.z() - min_pos.z()));

    openvdb::math::Mat4d mat = openvdb::math::Mat4d::identity();
    mat.setToScale(openvdb::math::Vec3(scale_x, scale_y, scale_z));
    mat.setTranslation(openvdb::math::Vec3(translate_x, translate_y, translate_z));

    openvdb::initialize();
    openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create();
    openvdb::math::Transform::Ptr trafo = openvdb::math::Transform::createLinearTransform(mat);
    grid->setTransform(trafo);

    openvdb::FloatGrid::Accessor accessor = grid->getAccessor();

    for(int x = -res.x() / 2; x <= res.x() / 2; x++) {
        for (int y = -res.y() / 2; y <= res.y() / 2; y++) {
            for (int z = -res.z() / 2; z <= res.z() / 2; z++) {
                openvdb::Coord xyz(x, y, z);
                openvdb::math::Vec3 pos = trafo->indexToWorld(xyz);

                int occurrences = count(positions, pos, metadata.radius);
                accessor.setValue(xyz, std::max(0, occurrences));
            }
        }
    }


    grid->setGridClass(openvdb::GRID_LEVEL_SET);
    // Name the grid "LevelSetSphere".
    grid->setName("density");
    // Create a VDB file object.
    openvdb::io::File file(metadata.out_path);
    // Add the grid pointer to a container.
    openvdb::GridPtrVec grids;
    grids.push_back(grid);
    // Write out the contents of the container.
    file.write(grids);
    file.close();
}

void write_vdb(const std::vector<Point3f>& main_positions, const std::vector<Point3f>& bg_positions, const ProceduralMetadata& metadata) {
    pcg32 random;

    Vector3i res = metadata.resolution;

    Vector3f min_pos = metadata.bounds.min;
    Vector3f max_pos = metadata.bounds.max;

    float scale_x = (max_pos.x() - min_pos.x()) / res.x();
    float scale_y = (max_pos.y() - min_pos.y()) / res.y();
    float scale_z = (max_pos.z() - min_pos.z()) / res.z();

    float translate_x = (max_pos.x() - min_pos.x()) * (0.5 + min_pos.x() / (max_pos.x() - min_pos.x()));
    float translate_y = (max_pos.y() - min_pos.y()) * (0.5 + min_pos.y() / (max_pos.y() - min_pos.y()));
    float translate_z = (max_pos.z() - min_pos.z()) * (0.5 + min_pos.z() / (max_pos.z() - min_pos.z()));

    openvdb::math::Mat4d mat = openvdb::math::Mat4d::identity();
    mat.setToScale(openvdb::math::Vec3(scale_x, scale_y, scale_z));
    mat.setTranslation(openvdb::math::Vec3(translate_x, translate_y, translate_z));

    openvdb::initialize();
    openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create();
    openvdb::math::Transform::Ptr trafo = openvdb::math::Transform::createLinearTransform(mat);
    grid->setTransform(trafo);

    openvdb::FloatGrid::Accessor accessor = grid->getAccessor();

    for(int x = -res.x() / 2; x <= res.x() / 2; x++) {
        for (int y = -res.y() / 2; y <= res.y() / 2; y++) {
            for (int z = -res.z() / 2; z <= res.z() / 2; z++) {
                openvdb::Coord xyz(x, y, z);
                openvdb::math::Vec3 pos = trafo->indexToWorld(xyz);

                int main_occurrences = count(main_positions, pos, metadata.radius);
                int bg_occurrences = count(bg_positions, pos, metadata.radius_bg);


                accessor.setValue(xyz, std::max(0.f, main_occurrences + 0.05f * bg_occurrences));
            }
        }
    }


    grid->setGridClass(openvdb::GRID_LEVEL_SET);
    // Name the grid "LevelSetSphere".
    grid->setName("density");
    // Create a VDB file object.
    openvdb::io::File file(metadata.out_path);
    // Add the grid pointer to a container.
    openvdb::GridPtrVec grids;
    grids.push_back(grid);
    // Write out the contents of the container.
    file.write(grids);
    file.close();
}


NORI_NAMESPACE_END