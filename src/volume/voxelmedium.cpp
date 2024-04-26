#include <core/common.h>
#include <volume/medium.h>
#include <volume/coefficient.h>

NORI_NAMESPACE_BEGIN

class VoxelMedium : public Medium {

public:
    explicit VoxelMedium(const PropertyList &props) {
        trafo = props.getTransform("toWorld", Transform());
        phase = new HenyeyGreensteinPhase(0.877f);
        absorption = new VoxelReader(props.getString("voxel_path"), trafo, props.getFloat("a_factor", 1.f));
        //absorption = new ConstantCoefficient(props.getFloat("d_factor"));
        scattering = new VoxelReader(props.getString("voxel_path"), trafo, props.getFloat("s_factor", 1.f));
    }

    std::string toString() const override {
        return "voxel";
    }

    BoundingBox3f bounds() const override {
        //return BoundingBox3f(Point3f(-1,-1,-1), Point3f(1,1,1));
        return static_cast<VoxelReader*>(absorption)->world_bounds();
    }

protected:
    Transform trafo;
};

NORI_REGISTER_CLASS(VoxelMedium, "voxel");

NORI_NAMESPACE_END