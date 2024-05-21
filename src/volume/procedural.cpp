#include <volume/procedural.h>
#include <openvdb/openvdb.h>
#include <openvdb/tools/Interpolation.h>
#include <pcg32.h>
#include <stats/warp.h>

NORI_NAMESPACE_BEGIN

using PointData = std::pair<Point3f, float>;

static int count(const std::vector<Point3f>& positions, const openvdb::math::Vec3<double>& v, float radius) {
    int c = 0;
    float x = v.x(), y = v.y(), z = v.z();
    for(auto p : positions) {
        float x0 = p.x(), y0 = p.y(), z0 = p.z();

        if((x0 - x) * (x0 - x) + (y0 - y) * (y0 - y) + (z0 - z) * (z0 - z) < radius * radius) {
            c++;
        }
    }
    return c;
}

static int count(const std::vector<PointData>& positions, const openvdb::math::Vec3<double>& v) {
    int c = 0;
    for(auto p : positions) {
        float x0 = p.first.x(), y0 = p.first.y(), z0 = p.first.z();
        float x = v.x(), y = v.y(), z = v.z();
        if((x0 - x) * (x0 - x) + (y0 - y) * (y0 - y) + (z0 - z) * (z0 - z) < p.second * p.second) {
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

void write_spiral(const Vector3i& res, const Transform& curve_transform, const std::string& output_path) {


    BoundingBox3f bounds;
    std::vector<PointData> points;

    int N_SPHERES = 8000;
    float STARTING_RADIUS = 20.f;
    float RADIUS_INCR = 9.0f;
    int N_REVOLUTIONS = 4;
    float SPHERE_RADIUS = 0.8f;
    float RADIUS_STD = 0.7f;
    float DISPLACEMENT_STD = 0.5f;

    pcg32 random;

    auto curve = [&](float t) {
        float radius = STARTING_RADIUS + RADIUS_INCR * t;
        float x = (radius + random.nextFloat()) * cos(2 * M_PI * t);
        float y = (radius + random.nextFloat()) * sin(2 * M_PI * t);
        return curve_transform * Point3f(x, 0, y);
    };


    for (int i = 0; i < N_SPHERES; i++) {
        Point3f p = curve(N_REVOLUTIONS * i / (float)N_SPHERES);
        float rad = (1 + Warp::lineToLogistic(random.nextFloat(), RADIUS_STD)) * SPHERE_RADIUS;
        float dx = Warp::lineToLogistic(random.nextFloat(), DISPLACEMENT_STD);
        float dy = Warp::lineToLogistic(random.nextFloat(), DISPLACEMENT_STD);
        float dz = Warp::lineToLogistic(random.nextFloat(), DISPLACEMENT_STD);
        Point3f point = Point3f (p.x() + dx, p.y() + dy, p.z() + dz);

        points.emplace_back(point, rad);
        bounds.expandBy(1.1 * point);
    }

    // Write VDB
    Vector3f min_pos = bounds.min;
    Vector3f max_pos = bounds.max;

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

                int occurrences = count(points, pos);
                accessor.setValue(xyz, std::max(0, occurrences));
            }
        }
    }


    grid->setGridClass(openvdb::GRID_LEVEL_SET);
    // Name the grid "LevelSetSphere".
    grid->setName("density");
    // Create a VDB file object.
    openvdb::io::File file(output_path);
    // Add the grid pointer to a container.
    openvdb::GridPtrVec grids;
    grids.push_back(grid);
    // Write out the contents of the container.
    file.write(grids);
    file.close();


}

static bool is_within_bounds(const Point3f& p, const std::vector<Point3f>& bounds, float radius) {
    for(const auto& c : bounds) {
        float px = p.x(), py = p.y(), pz = p.z();
        float cx = c.x(), cy = c.y(), cz = c.z();
        if((px-cx)*(px-cx) + (py-cy)*(py-cy) + (pz-cz)*(pz-cz) < radius*radius) {
            return true;
        }
    }
    return false;
}

static Vector3f random_displacement(pcg32& random, bool ultra_wide) {
    if(ultra_wide) {
        float dx = 2 * random.nextFloat() - 1;
        float dy = 2 * random.nextFloat() - 1;
        float dz = 2 * random.nextFloat() - 1;
        return Point3f(dx, dy, dz);
    }
    // Biais towards low radius on purpose
    float dphi = 2 * M_PI * random.nextFloat();
    float dtheta = M_PI * random.nextFloat();
    return pow(random.nextFloat(), 0.5f) * sphericalDirection(dtheta, dphi);
}

void write_sky(const Vector3i& n_clouds, const Vector3i& voxel_res, const BoundingBox3f& bounds, const Point3f& hole,
               float hole_radius, const std::string& output_path) {

    // Add a few initial big spheres that make the main shape (instead of just square)
    pcg32 random;

    std::vector<Point3f> points;

    float min_x = bounds.min.x();
    float min_y = bounds.min.y();
    float min_z = bounds.min.z();
    float max_x = bounds.max.x();
    float max_y = bounds.max.y();
    float max_z = bounds.max.z();

    float size_x = (max_x - min_x) / n_clouds.x();
    float size_y = (max_y - min_y) / n_clouds.y();
    float size_z = (max_z - min_z) / n_clouds.z();

    bool ultra_wide = (size_x + size_z) / (2.0 * size_y) > 20.0;

    std::cout << "size_x=" << size_x << ", size_y=" << size_y << ", size_z=" << size_z << ", ultrawide: " << std::to_string(ultra_wide) << "\n";

    float OFFSET_DAMPING = 0.1;
    int N_SPHERES_PER_CLOUD = 2000 / (n_clouds.x() * n_clouds.y() * n_clouds.z());
    float SPHERE_RADIUS = std::max({size_x, size_y, size_z}) * 0.0225;
    float BOUNDING_SPHERE_RADIUS = std::max({size_x, size_y, size_z}) * 0.25;
    int N_BOUNDING_SPHERES_PER_CLOUD = std::max({size_x, size_y, size_z}) * 0.02;

    for(int rx = 0; rx < n_clouds.x(); rx++) {
        for(int ry = 0; ry < n_clouds.y(); ry++) {
            for(int rz = 0; rz < n_clouds.z(); rz++) {

                float cx = min_x + (rx + 0.5) * size_x;
                float cy = min_y + (ry + 0.5) * size_y;
                float cz = min_z + (rz + 0.5) * size_z;

                float offset_x = OFFSET_DAMPING * random.nextFloat() * size_x;
                float offset_y = OFFSET_DAMPING * random.nextFloat() * size_y;
                float offset_z = OFFSET_DAMPING * random.nextFloat() * size_z;

                std::vector<Point3f> cloud_bounds(N_BOUNDING_SPHERES_PER_CLOUD);

                for(int i = 0; i < N_BOUNDING_SPHERES_PER_CLOUD; i++) {
                    Vector3f displacement = random_displacement(random, ultra_wide);
                    float x = cx + ((size_x - offset_x) / 2.0) * displacement.x();
                    float y = cy + ((size_y - offset_y) / 2.0) * displacement.y();
                    float z = cz + ((size_z - offset_z) / 2.0) * displacement.z();
                    cloud_bounds[i] = Point3f(x,y,z);
                }

                for(int i = 0; i < N_SPHERES_PER_CLOUD; i++) {
                    Vector3f displacement = random_displacement(random, ultra_wide);
                    float x = cx + ((size_x - offset_x) / 2.0) * displacement.x();
                    float y = cy + ((size_y - offset_y) / 2.0) * displacement.y();
                    float z = cz + ((size_z - offset_z) / 2.0) * displacement.z();
                    if(is_within_bounds(Point3f(x,y,z), cloud_bounds, BOUNDING_SPHERE_RADIUS)){

                        points.emplace_back(x, y, z);
                    }
                }
            }
        }
    }

    float scale_x = (max_x - min_x) / voxel_res.x();
    float scale_y = (max_y - min_y) / voxel_res.y();
    float scale_z = (max_z - min_z) / voxel_res.z();

    float translate_x = (max_x - min_x) * (0.5 + min_x / (max_x - min_x));
    float translate_y = (max_y - min_y) * (0.5 + min_y / (max_y - min_y));
    float translate_z = (max_z - min_z) * (0.5 + min_z / (max_z - min_z));

    openvdb::math::Mat4d mat = openvdb::math::Mat4d::identity();
    mat.setToScale(openvdb::math::Vec3(scale_x, scale_y, scale_z));
    mat.setTranslation(openvdb::math::Vec3(translate_x, translate_y, translate_z));

    openvdb::initialize();
    openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create();
    openvdb::math::Transform::Ptr trafo = openvdb::math::Transform::createLinearTransform(mat);
    grid->setTransform(trafo);

    openvdb::FloatGrid::Accessor accessor = grid->getAccessor();

    for(int x = -voxel_res.x() / 2; x <= voxel_res.x() / 2; x++) {
        for (int y = -voxel_res.y() / 2; y <= voxel_res.y() / 2; y++) {
            for (int z = -voxel_res.z() / 2; z <= voxel_res.z() / 2; z++) {
                openvdb::Coord xyz(x, y, z);
                openvdb::math::Vec3 pos = trafo->indexToWorld(xyz);

                float cx = pos.x(), cy = pos.y(), cz = pos.z();


                int main_occurrences = count(points, pos, SPHERE_RADIUS);

                if((cx - hole.x()) * (cx - hole.x()) + (cy - hole.y()) * (cy - hole.y()) + (cz - hole.z()) * (cz - hole.z()) < hole_radius * hole_radius &&
                   (cx - hole.x()) * (cx - hole.x()) + (cy - hole.y()) * (cy - hole.y()) + (cz - hole.z()) * (cz - hole.z()) > -0.1 * hole_radius * hole_radius) {
                    main_occurrences /= 10;
                }
                accessor.setValue(xyz, main_occurrences);
            }
        }
    }


    grid->setGridClass(openvdb::GRID_LEVEL_SET);
    grid->setName("density");
    // Create a VDB file object.
    openvdb::io::File file(output_path);
    // Add the grid pointer to a container.
    openvdb::GridPtrVec grids;
    grids.push_back(grid);
    // Write out the contents of the container.
    file.write(grids);
    file.close();
}

NORI_NAMESPACE_END