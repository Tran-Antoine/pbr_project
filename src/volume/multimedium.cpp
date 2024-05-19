#include <core/common.h>
#include <volume/medium.h>
#include <volume/coefficient.h>
#include <core/color.h>

NORI_NAMESPACE_BEGIN

class MultiVoxelMedium : public Medium {

public:
    explicit MultiVoxelMedium(const PropertyList &props) {
        trafo = props.getTransform("toWorld", Transform());
    }

    void activate() override {

        Medium::activate();

        std::vector<MediumCoefficient*> coeffsAbs(children.size());
        std::vector<MediumCoefficient*> coeffsSca(children.size());

        for (int i = 0; i < children.size(); i++) {
            coeffsAbs[i] = children[i]->getAbsorption();
            coeffsSca[i] = children[i]->getScattering();
            children_bounds.push_back(children[i]->bounds());
            w_bounds.expandBy(children[i]->bounds());
        }

        absorption = new MultiMediumCoefficient(coeffsAbs, children_bounds);
        scattering = new MultiMediumCoefficient(coeffsSca, children_bounds);
    }

    void addChild(NoriObject *obj) override {
        switch (obj->getClassType()) {
            case NoriObject::EMedium:
                children.push_back(static_cast<Medium *>(obj));
                break;
            default:
                Medium::addChild(obj);
        }
    }

    std::string toString() const override {
        return "mvoxel";
    }

    float majorant(const Ray3f& ray) const override {
        float maj = 0;
        for (auto c : children) {
            if(c->bounds().rayIntersect(ray)) {
                if(c->majorant(ray) > maj) {
                    maj = c->majorant(ray);
                }
            }
        }
        return maj;
    }

    BoundingBox3f bounds() const override {
        return w_bounds;
    }

    std::vector<BoundingBox3f> n_bounds() const override {
        return children_bounds;
    }

protected:
    Transform trafo;
    std::vector<Medium*> children;
    std::vector<BoundingBox3f> children_bounds;
    BoundingBox3f w_bounds;
};

NORI_REGISTER_CLASS(MultiVoxelMedium, "mvoxel");

NORI_NAMESPACE_END