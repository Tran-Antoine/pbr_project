#include <core/common.h>
#include <volume/medium.h>
#include <volume/coefficient.h>
#include <core/mesh.h>

NORI_NAMESPACE_BEGIN

class HomogeneousMedium : public Medium {

public:
    explicit HomogeneousMedium(const PropertyList &props) {

        absorption = new ConstantCoefficient(props.getColor("absorption"));
        scattering = new ConstantCoefficient(props.getColor("scattering"));
        Point3f a = props.getPoint("bounds_min", 0);
        Point3f b = props.getPoint("bounds_max", 0);

        if(!(a.isZero() && b.isZero())) {
            Point3f min = Point3f(std::min(a.x(), b.x()), std::min(a.y(), b.y()), std::min(a.z(), b.z()));
            Point3f max = Point3f(std::max(a.x(), b.x()), std::max(a.y(), b.y()), std::max(a.z(), b.z()));
            w_bounds = BoundingBox3f(min, max);
        }
    }

    void addChild(nori::NoriObject *obj) override {
        BoundingBox3f bbox;
        Point3f a, b, min, max;
        switch (obj->getClassType()) {
            case NoriObject::EMesh:
                bbox = static_cast<Mesh*>(obj)->getBoundingBox();
                a = bbox.min;
                b = bbox.max;
                min = Point3f(std::min(a.x(), b.x()), std::min(a.y(), b.y()), std::min(a.z(), b.z()));
                max = Point3f(std::max(a.x(), b.x()), std::max(a.y(), b.y()), std::max(a.z(), b.z()));
                w_bounds = BoundingBox3f(min, max);
                break;
            default:
                Medium::addChild(obj);
        }
    }

    void setParent(nori::NoriObject *parent) override {
        if(parent->getClassType() == EClassType::EMesh) {
            BoundingBox3f bbox = static_cast<Mesh*>(parent)->getBoundingBox();
            Point3f a = bbox.min;
            Point3f b = bbox.max;
            Point3f min = Point3f(std::min(a.x(), b.x()), std::min(a.y(), b.y()), std::min(a.z(), b.z()));
            Point3f max = Point3f(std::max(a.x(), b.x()), std::max(a.y(), b.y()), std::max(a.z(), b.z()));
            w_bounds = BoundingBox3f(min, max);
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