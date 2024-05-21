#include <core/common.h>
#include <core/color.h>
#include <bsdf/multidiffusemap.h>

NORI_NAMESPACE_BEGIN

MultiDiffuseMap::MultiDiffuseMap(const PropertyList &propList) : DiffuseMap(propList) { }

Color3f MultiDiffuseMap::T(float s, float t) {
    int index;
    float remapped;
    unmap(s, remapped, index);

    return children[index]->T(remapped, t);
}

std::string MultiDiffuseMap::toString() const {
    return "multitexturemap";
}

void MultiDiffuseMap::addChild(nori::NoriObject *obj) {
    DiffuseMap* map;
    switch (obj->getClassType()) {
        case NoriObject::EDiffuseMap:
            map = static_cast<DiffuseMap *>(obj);
            children.push_back(map);
            break;
        default:
            NoriObject::addChild(obj);
            break;
    }
}

float MultiDiffuseMap::map(float s, int index) const {
    float range = 1.f / children.size();
    float start = index * range;
    return start + s * range;
}

void MultiDiffuseMap::unmap(float m, float& s, int& index) const {
    float range = 1.f / children.size();

    index = clamp(((int) (m / range)), 0, children.size() - 1);
    s = m/range - index; //  (m - index * range) / range
}

NORI_REGISTER_CLASS(MultiDiffuseMap, "mtexture");
NORI_NAMESPACE_END