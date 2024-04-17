#include <openvdb/openvdb.h>
#include <openvdb/tools/ChangeBackground.h>
#include <iostream>
#include <openvdb/tools/Interpolation.h>

static std::string PATH = "assets/voxel/Cloud_02.vdb";

int main() {
    openvdb::initialize();
    // Create a VDB file object.
    openvdb::io::File file(PATH);
    // Open the file.  This reads the file header, but not any grids.
    file.open();

    openvdb::GridBase::Ptr baseGrid;
    for (openvdb::io::File::NameIterator nameIter = file.beginName();
         nameIter != file.endName(); ++nameIter) {
        // Read in only the grid we are interested in.
        if (nameIter.gridName() == "density") {
            baseGrid = file.readGrid(nameIter.gridName());
        } else {
            std::cout << "skipping grid " << nameIter.gridName() << std::endl;
        }
    }
    file.close();

    using GridType = openvdb::FloatGrid;

    const GridType::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);

    openvdb::tools::GridSampler<GridType, openvdb::tools::BoxSampler> sampler(*grid);

    for (openvdb::MetaMap::MetaIterator iter = grid->beginMeta();
         iter != grid->endMeta(); ++iter) {
        const std::string& name = iter->first;
        openvdb::Metadata::Ptr value = iter->second;
        std::string valueAsString = value->str();
        std::cout << name << " = " << valueAsString << " (" << value->typeName() << ")" <<  std::endl;
    }

    std::cout << grid->indexToWorld(openvdb::Coord(300, 0, 0)) << "\n";

    for (openvdb::FloatGrid::ValueOnCIter iter = grid->cbeginValueOn(); iter; ++iter) {
        //std::cout << "Grid" << iter.getCoord() << " = " << *iter << std::endl;
    }
}