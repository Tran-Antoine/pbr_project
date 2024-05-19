#include <core/common.h>
#include <volume/medium.h>
#include <volume/coefficient.h>
#include <core/color.h>

NORI_NAMESPACE_BEGIN

class PoolVoxelMedium : public Medium {

public:
    explicit PoolVoxelMedium(const PropertyList &props) {

        model_path = props.getString("model_path");
        Point3f a = props.getPoint("bounds_min", 0);
        Point3f b = props.getPoint("bounds_max", 0);
        Point3f min = Point3f(std::min(a.x(), b.x()), std::min(a.y(), b.y()), std::min(a.z(), b.z()));
        Point3f max = Point3f(std::max(a.x(), b.x()), std::max(a.y(), b.y()), std::max(a.z(), b.z()));
        w_bounds = BoundingBox3f(min, max);
    }

    void activate() override {

        Medium::activate();

        float min_x = w_bounds.min.x();
        float min_y = w_bounds.min.y();
        float min_z = w_bounds.min.z();
        float max_x = w_bounds.max.x();
        float max_y = w_bounds.max.y();
        float max_z = w_bounds.max.z();


        for(int rx = 0; rx < resolution.x(); rx++) {
            for(int ry = 0; ry < resolution.y(); ry++) {
                for(int rz = 0; rz < resolution.z(); rz++) {

                    float cx = min_x + rx * (max_x - min_x) / resolution.x();
                    float cy = min_y + ry * (max_y - min_y) / resolution.y();
                    float cz = min_z + rz * (max_z - min_z) / resolution.z();

                }
            }
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

    std::string toString() const override {
        return "pvoxel";
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
    std::vector<Medium*> children;
    BoundingBox3f w_bounds;
    Vector3i resolution;
    std::string model_path;
};

NORI_REGISTER_CLASS(PoolVoxelMedium, "pvoxel");

NORI_NAMESPACE_END