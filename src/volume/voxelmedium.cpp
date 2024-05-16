#include <core/common.h>
#include <volume/medium.h>
#include <volume/coefficient.h>
#include <core/color.h>
#include <volume/procedural.h>

NORI_NAMESPACE_BEGIN

class VoxelMedium : public Medium {

public:
    explicit VoxelMedium(const PropertyList &props) {

        trafo = props.getTransform("toWorld", Transform());
        auto path = props.getString("voxel_path");

        write_spiral(Vector3i(120, 40, 120), trafo, path);

        absorption = new VoxelReader(path, Transform(), props.getColor("absorption"));
        scattering = new VoxelReader(path, Transform(), props.getColor("scattering"));
    }

    std::string toString() const override {
        return "voxel";
    }

    BoundingBox3f bounds() const override {
        return static_cast<VoxelReader*>(absorption)->world_bounds();
    }

protected:
    Transform trafo;
};

NORI_REGISTER_CLASS(VoxelMedium, "voxel");

NORI_NAMESPACE_END