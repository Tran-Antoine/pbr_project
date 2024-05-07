#include <iostream>
#include <volume/coefficient.h>
#include <core/transform.h>
#include <core/vector.h>

static std::string PATH = "assets/voxel/procedural/test.vdb";

using namespace nori;

int main() {


    openvdb::initialize();
    openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create();

    openvdb::math::Mat4d mat = openvdb::math::Mat4d::identity();
    mat.setToScale(openvdb::math::Vec3(0.1, 0.1, 0.1));
    openvdb::math::Transform::Ptr vdb_transform = openvdb::math::Transform::createLinearTransform(mat);
    grid->setTransform(vdb_transform);


    openvdb::FloatGrid::Accessor accessor = grid->getAccessor();

    for(int x = -10; x <= 10; x++) {
        for(int y = -10; y <= 10; y++) {
            for(int z = -10; z <= 10; z++) {
                openvdb::Coord xyz(x,y,z);
                accessor.setValue(xyz, (y+10.0)/10.0);
            }
        }
    }

    for (openvdb::FloatGrid::ValueOnCIter iter = grid->cbeginValueOn(); iter; ++iter) {
        std::cout << "Positions" << grid->transform().indexToWorld(iter.getCoord()) << " = " << *iter << std::endl;
    }

    grid->setGridClass(openvdb::GRID_LEVEL_SET);
    // Name the grid "LevelSetSphere".
    grid->setName("density");
    // Create a VDB file object.
    openvdb::io::File file(PATH);
    // Add the grid pointer to a container.
    openvdb::GridPtrVec grids;
    grids.push_back(grid);
    // Write out the contents of the container.
    file.write(grids);
    file.close();
}