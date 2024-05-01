#include <core/common.h>
#include <volume/medium.h>
#include <volume/coefficient.h>

NORI_NAMESPACE_BEGIN

class VoxelMedium : public Medium {

public:
    explicit VoxelMedium(const PropertyList &props) {
        trafo = props.getTransform("toWorld", Transform());
        phase = new HenyeyGreensteinPhase(0.877);
        absorption = new VoxelReader(props.getString("voxel_path"), trafo, props.getFloat("a_factor", 1.f));
        //scattering = new ScatteringVoxelReader(props.getFloat("s_factor", 1.f), new VoxelReader(props.getString("voxel_path"), trafo, props.getFloat("s_factor", 1.f)));
        //scattering = new BinaryVoxelReader(0.01f, new VoxelReader(props.getString("voxel_path"), trafo, props.getFloat("s_factor", 1.f)));
        scattering = new VoxelReader(props.getString("voxel_path"), trafo, props.getFloat("s_factor", 1.f));
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