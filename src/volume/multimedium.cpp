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

        if(!phase) {
            phase = new HenyeyGreensteinPhase(0.877);
        }

        std::vector<MediumCoefficient*> coeffsAbs(children.size());
        std::vector<MediumCoefficient*> coeffsSca(children.size());
        std::vector<BoundingBox3f> boundsAbs(children.size());
        std::vector<BoundingBox3f> boundsSca(children.size());

        for (int i = 0; i < children.size(); i++) {
            coeffsAbs[i] = children[i]->getAbsorption();
            coeffsSca[i] = children[i]->getScattering();
            boundsAbs[i] = children[i]->bounds();
            boundsSca[i] = children[i]->bounds();
            w_bounds.expandBy(children[i]->bounds());
        }

        absorption = new MultiMediumCoefficient(coeffsAbs, boundsAbs);
        scattering = new MultiMediumCoefficient(coeffsSca, boundsSca);
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

protected:
    Transform trafo;
    std::vector<Medium*> children;
    BoundingBox3f w_bounds;
};

NORI_REGISTER_CLASS(MultiVoxelMedium, "mvoxel");

NORI_NAMESPACE_END