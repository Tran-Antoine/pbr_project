#include <core/common.h>
#include <volume/medium.h>
#include <volume/coefficient.h>

NORI_NAMESPACE_BEGIN

class VoxelMedium : public Medium {

public:
    explicit VoxelMedium(const PropertyList &props) {
        trafo = props.getTransform("toWorld", Transform());
        phase = new UniformPhase();
        absorption = new VoxelReader(props.getString("voxel_path"), trafo);
        scattering = new ConstantCoefficient(props.getFloat("scattering"));
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