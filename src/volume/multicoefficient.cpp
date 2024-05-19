#include <core/common.h>
#include <volume/coefficient.h>

NORI_NAMESPACE_BEGIN

MultiMediumCoefficient::MultiMediumCoefficient(const std::vector<MediumCoefficient*>& children, const std::vector<BoundingBox3f>& children_bounds)
    : children(children),
      children_bounds(children_bounds) { }

BoundingBox3i MultiMediumCoefficient::index_bounds() const {
    std::cout << "Function index bounds not implemented" << std::endl;
    throw NoriException("Function index bounds not implemented");
}

BoundingBox3f MultiMediumCoefficient::world_bounds() const {
    std::cout << "Function world bounds not implemented" << std::endl;
    throw NoriException("Function world bounds not implemented");
}

float MultiMediumCoefficient::maj(const Ray3f& ray) const {
    std::cout << "Function maj not implemented" << std::endl;
    throw NoriException("Function maj not implemented");
}

Color3f MultiMediumCoefficient::eval(const nori::Point3f &p, const nori::Vector3f &v) const {
    Color3f out(0.f);
    for(int i = 0; i < children.size(); i++) {
        auto child = children[i];
        auto bounds = children_bounds[i];
        if(bounds.contains(p)) {
            Color3f current = child->eval(p, v);
            out.r() = std::max(out.r(), current.r());
            out.g() = std::max(out.g(), current.g());
            out.b() = std::max(out.b(), current.b());
        }
    }
    return out;
}

NORI_NAMESPACE_END