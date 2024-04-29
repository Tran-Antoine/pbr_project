#pragma once

#include <core/common.h>
#include <openvdb/openvdb.h>

NORI_NAMESPACE_BEGIN

inline openvdb::FloatGrid::Ptr from_file(const std::string& path, const Transform& transform) {
    openvdb::initialize();
    // Create a VDB file object.
    openvdb::io::File file(path);
    // Open the file.  This reads the file header, but not any grids.
    file.open();

    openvdb::GridBase::Ptr baseGrid;
    for (openvdb::io::File::NameIterator nameIter = file.beginName();
         nameIter != file.endName(); ++nameIter) {
        // Read in only the grid we are interested in.
        if (nameIter.gridName() == "density") {
            baseGrid = file.readGrid(nameIter.gridName());
        } else {
            std::cout << "Warning: skipped grid " << nameIter.gridName() << std::endl;
        }
    }
    file.close();

    auto tmatrix = transform.getMatrix();

    openvdb::math::Vec4d c0(tmatrix.col(0).x(), tmatrix.col(0).y(), tmatrix.col(0).z(), tmatrix.col(0).w());
    openvdb::math::Vec4d c1(tmatrix.col(1).x(), tmatrix.col(1).y(), tmatrix.col(1).z(), tmatrix.col(1).w());
    openvdb::math::Vec4d c2(tmatrix.col(2).x(), tmatrix.col(2).y(), tmatrix.col(2).z(), tmatrix.col(2).w());
    openvdb::math::Vec4d c3(tmatrix.col(3).x(), tmatrix.col(3).y(), tmatrix.col(3).z(), tmatrix.col(3).w());

    openvdb::math::Mat4d mat = openvdb::math::Mat4d::identity();
    mat.setColumns(c0, c1, c2, openvdb::math::Vec4d(0,0,0,1));
    openvdb::math::Transform::Ptr vdb_transform = openvdb::math::Transform::createLinearTransform(mat);

    vdb_transform->postTranslate(c3.getVec3());
    baseGrid->setTransform(vdb_transform);

    return openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
}

NORI_NAMESPACE_END