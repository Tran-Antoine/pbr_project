#include <core/common.h>
#include <volume/medium.h>
#include <volume/coefficient.h>
#include <core/mesh.h>

NORI_NAMESPACE_BEGIN

class HomogeneousMedium : public Medium {

public:
    explicit HomogeneousMedium(const PropertyList &props) {
        phase = new UniformPhase();
        absorption = new ConstantCoefficient(props.getFloat("absorption"));
        scattering = new ConstantCoefficient(props.getFloat("scattering"));
        Point3f min = props.getPoint("bounds_min", 0);
        Point3f max = props.getPoint("bounds_max", 0);
        if(!(min.isZero() && max.isZero())) {
            w_bounds = BoundingBox3f(min, max);
        }
    }

    void setParent(nori::NoriObject *parent) override {
        if(parent->getClassType() == EClassType::EMesh) {
            w_bounds = static_cast<Mesh*>(parent)->getBoundingBox();
        }
    }

    std::string toString() const override {
        return "homogeneous";
    }

    BoundingBox3f bounds() const override {
        return w_bounds;
    }

protected:
    BoundingBox3f w_bounds;
};

NORI_REGISTER_CLASS(HomogeneousMedium, "homogeneous");
NORI_NAMESPACE_END