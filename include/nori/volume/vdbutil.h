#pragma once

#include <core/common.h>
#include <openvdb/openvdb.h>

NORI_NAMESPACE_BEGIN

inline openvdb::FloatGrid::Ptr from_file(const std::string& path) {
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

    return openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
}

NORI_NAMESPACE_END