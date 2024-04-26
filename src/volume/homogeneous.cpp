#include <core/common.h>
#include <volume/medium.h>
#include <volume/coefficient.h>

NORI_NAMESPACE_BEGIN

class HomogeneousMedium : public Medium {

public:
    explicit HomogeneousMedium(const PropertyList &props) {
        phase = new UniformPhase();
        absorption = new ConstantCoefficient(props.getFloat("absorption"));
        scattering = new ConstantCoefficient(props.getFloat("scattering"));
    }

    std::string toString() const override {
        return "homogeneous";
    }

    BoundingBox3f bounds() const override {
        return BoundingBox3f(Point3f(-1), Point3f(1));
    }
};

NORI_REGISTER_CLASS(HomogeneousMedium, "homogeneous");
NORI_NAMESPACE_END